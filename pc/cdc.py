import serial
from serial.tools import list_ports
import time
from datetime import datetime, timedelta
import json

def get_available_cdc_ports():
    """
    获取所有可用的 USB CDC 端口
    """
    ports = list_ports.comports()
    return [port.device for port in ports]

def send_in_segments(data_bytes, ser, segment_size=32, delay=0.01):
    """
    按32字节分段发送 data_bytes，
    如果数据恰好能整除，则再追加一个只有一个 "\0" 的段，每段发送后延迟 delay 秒。
    """
    segments = []
    total_len = len(data_bytes)
    for i in range(0, total_len, segment_size):
        segment = data_bytes[i:i+segment_size]
        segments.append(segment)
    if total_len % segment_size == 0:
        segments.append(b'\0')
    for idx, seg in enumerate(segments):
        # 循环发送当前段，直到反馈不包含 busy
        while True:
            ser.write(seg)
            resp = ser.read_all().decode("utf-8", errors="replace")
            if "busy" in resp.lower():
                print("检测到 busy，重发当前消息...")
                time.sleep(delay)
            else:
                break
        if idx < len(segments) - 1:
            time.sleep(delay)

def wait_for_signal(ser, target, timeout=None):
    """
    若 timeout 为 None，则无限等待；否则在指定 timeout 内等待 ser 回传中包含 target 字符串。
    检测到 target 时返回 True，否则超时返回 False。
    """
    start_time = datetime.now()
    while True:
        resp = ser.read_all().decode("utf-8", errors="replace")
        if target in resp:
            print(f"检测到信号: {target}")
            return True
        if timeout is not None and (datetime.now() - start_time) > timedelta(seconds=timeout):
            return False
        time.sleep(0.005)

def send_to_cdc(out_json, com_port, baudrate=9600):
    """
    修改后的逻辑：
      1. 首先将原 JSON 中 segments 内 characters 清空后发送；
      2. 然后逐个发送 characters 内的项，每发送完一个后立即停止传输，直到检测到对方传来 "114514" 才继续发送下一个；
      3. 最后发送完所有 character 后，若对方再次传来 "114514"，则发送 "810" 表示此次写字结束。
    
    返回 (success, message) tuple
    """
    try:
        ser = serial.Serial(port=com_port, baudrate=baudrate, timeout=1)
        # 解析原始 JSON 数据
        original_data = json.loads(out_json)
        characters = original_data.get("segments", [{}])[0].get("characters", [])
        # 制作初始消息：将 segments 内 characters 数组置空
        modified_data = original_data.copy()
        if "segments" in modified_data and len(modified_data["segments"]) > 0:
            modified_data["segments"][0]["characters"] = []
        else:
            return False, "JSON 格式异常：缺少 segments"
        init_json = json.dumps(modified_data, ensure_ascii=False, separators=(',',':'))
        # 发送初始消息
        data_bytes = init_json.encode("utf-8")
        send_in_segments(data_bytes, ser, segment_size=32, delay=0.01)
        # 发送初始消息后等待对方发送 "114514" 信号，再开始发送第1个 character
        print("已发送初始消息，等待 '114514' 信号以开始发送第 1 个 character ...")
        wait_for_signal(ser, "114514", timeout=None)
        
        # 直接进入发送每个 character 的流程
        for idx, char_item in enumerate(characters):
            json_message = json.dumps(char_item, ensure_ascii=False, separators=(',',':'))
            # print(json_message)
            data_bytes = json_message.encode("utf-8")
            send_in_segments(data_bytes, ser, segment_size=32, delay=0.01)
            # 发送完当前 character 后，等待对方传来"114514"以继续
            print(f"已发送第 {idx+1} 个character，等待 '114514' 信号以继续...")
            wait_for_signal(ser, "114514", timeout=None)
        
        # 发送完所有 character 后，发送 "1919810" 信号以结束写字
        print("所有 character 已发送，发送 '1919810' 信号以结束写字...")
        end_msg = "1919810"
        end_bytes = end_msg.encode("utf-8")
        send_in_segments(end_bytes, ser, segment_size=32, delay=0.01)
        
        ser.close()
        return True, f"数据已通过 {com_port} 发送"
    except Exception as e:
        return False, f"发送数据到 {com_port} 时出错: {e}"