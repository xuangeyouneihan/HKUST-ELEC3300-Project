import serial
from serial.tools import list_ports

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
        print(com_port)
        ser = serial.Serial(port=com_port, baudrate=baudrate, timeout=1)
        print(2)
        ser.write(out_json.encode("utf-8"))
        print(1)
        ser.close()
        return True, f"数据已通过 {com_port} 发送"
    except Exception as e:
        return False, f"发送数据到 {com_port} 时出错: {e}"