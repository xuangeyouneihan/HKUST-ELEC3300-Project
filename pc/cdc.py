import serial
from serial.tools import list_ports
import time
from datetime import datetime, timedelta

def get_available_cdc_ports():
    """
    获取所有可用的 USB CDC 端口
    """
    ports = list_ports.comports()
    return [port.device for port in ports]

def send_to_cdc(out_json, com_port, baudrate=9600):
    """
    通过指定的 USB CDC 端口发送 JSON 数据，每段大小均为32字节，
    末段若不够32字节后面用 "\0" 填全，若末段大小恰好为32字节则再追加
    一个全是 "\0" 的32字节段。发完一段延迟10ms再发送下一段。
    
    同时读取并打印 CDC 设备的回传值。
    
    返回 (success, message) tuple
    """
    try:
        ser = serial.Serial(port=com_port, baudrate=baudrate, timeout=1)
        data_bytes = out_json.encode("utf-8")
        segment_size = 32
        segments = []
        total_len = len(data_bytes)
        
        # 分段
        for i in range(0, total_len, segment_size):
            segment = data_bytes[i:i+segment_size]
            segments.append(segment)
            # print(segment)
        
        # 若数据正好被32整除，则再追加一段全 "\0" 的段
        if total_len % segment_size == 0:
            segments.append(b'\0')
        
        # 分段发送，每段结束后延迟10ms
        for idx, seg in enumerate(segments):
            # print(seg)
            ser.write(seg)
            print(ser.read_all().decode("utf-8", errors="replace"))
            if idx < len(segments) - 1:
                time.sleep(0.01)

        start_time = datetime.now()
        duration = timedelta(seconds=0.5)
        while datetime.now() - start_time < duration:
             print(ser.read_all().decode("utf-8", errors="replace"))
        ser.close()
        return True, f"数据已通过 {com_port} 发送"
    except Exception as e:
        return False, f"发送数据到 {com_port} 时出错: {e}"