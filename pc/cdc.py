import serial
from serial.tools import list_ports
import threading

def get_available_cdc_ports():
    """
    获取所有可用的 USB CDC 端口
    """
    ports = list_ports.comports()
    return [port.device for port in ports]

def send_to_cdc(out_json, com_port, baudrate=9600):
    """
    通过指定的 USB CDC 端口发送 JSON 数据

    返回 (success, message) tuple
    """
    try:
        ser = serial.Serial(port=com_port, baudrate=baudrate, timeout=1)
        ser.write(out_json.encode("utf-8"))
        ser.close()
        return True, f"数据已通过 {com_port} 发送"
    except Exception as e:
        return False, f"发送数据到 {com_port} 时出错: {e}"

def listen_to_cdc(com_port, baudrate=9600, data_callback=None):
    """
    启动一个后台线程，监听指定 CDC 端口
    data_callback: 当接收到数据时调用的回调函数，格式为 callback(event_type, data)
                   event_type 可能为 "data"（正常数据）、"error"（读取错误）或 "closed"（端口关闭）
    """
    try:
        ser = serial.Serial(port=com_port, baudrate=baudrate, timeout=1)
    except Exception as e:
        if data_callback:
            data_callback("error", f"打开串口时出错: {e}")
        return

    def read_loop():
        while True:
            try:
                if ser.in_waiting:
                    # 读取一行数据，假设设备以换行符结束数据
                    line = ser.readline().decode("utf-8", errors="replace").strip()
                    if line and data_callback:
                        data_callback("data", line)
            except Exception as e:
                if data_callback:
                    data_callback("error", f"读取数据时出错: {e}")
                break
        ser.close()
        if data_callback:
            data_callback("closed", "串口已关闭")

    thread = threading.Thread(target=read_loop, daemon=True)
    thread.start()