import tkinter as tk
import tkinter.font as tkfont
from tkinter import ttk, messagebox, filedialog
import json
import os
from matplotlib import font_manager
import numpy as np

# 引入已有的 prepare_writing_robot_data 函数
from font_process import prepare_writing_robot_data
from cdc import send_to_cdc, get_available_cdc_ports, listen_to_cdc

global root, paper_var, page_width_var, page_height_var, top_margin_var, bottom_margin_var
global left_margin_var, right_margin_var, font_size_var, font_family_var, font_path_var
global text_input, paper_menu, font_select_combo

# 使用 matplotlib 获取系统中所有字体，并构造字体名称到字体路径的映射字典
FONT_DICT = {f.name: f.fname for f in font_manager.fontManager.ttflist}
available_fonts = sorted(FONT_DICT.keys())

# 全局字体文件默认路径（这里默认使用映射字典中的 Arial 字体，如果存在）
BASE_DIR = os.path.dirname(__file__)
FONT_PATH = os.path.join(BASE_DIR, "ReliefSingleLineCAD-Regular.ttf")

# 纸张预设参数（单位：毫米）
PAPER_PRESETS = {
    "A4": {
        "page_width": 210,
        "page_height": 297,
        "top_margin": 25.4,
        "bottom_margin": 25.4,
        "left_margin": 25.4,
        "right_margin": 25.4,
    },
    "A5": {
        "page_width": 148,
        "page_height": 210,
        "top_margin": 20,
        "bottom_margin": 20,
        "left_margin": 20,
        "right_margin": 20,
    },
    "B5": {
        "page_width": 182,
        "page_height": 257,
        "top_margin": 22,
        "bottom_margin": 22,
        "left_margin": 22,
        "right_margin": 22,
    }
}

def update_margins(*args):
    """更新页面尺寸和页边距"""
    preset = paper_var.get()
    if preset in PAPER_PRESETS:
        config = PAPER_PRESETS[preset]
        top_margin_var.set(str(config["top_margin"]))
        bottom_margin_var.set(str(config["bottom_margin"]))
        left_margin_var.set(str(config["left_margin"]))
        right_margin_var.set(str(config["right_margin"]))
        page_width_var.set(str(config["page_width"]))
        page_height_var.set(str(config["page_height"]))

def get_font_file():
    """直接返回 font_path_var 中的字体文件路径，如果存在则返回，否则返回 None"""
    fallback_font = font_path_var.get()
    if os.path.exists(fallback_font):
        return fallback_font
    return None

def on_font_selection(event=None):
    """根据字体路径输入框更新全局变量 FONT_PATH，并自动转换为绝对路径"""
    candidate = font_path_var.get()
    candidate_abs = os.path.abspath(candidate)
    global FONT_PATH
    if os.path.exists(candidate_abs):
        FONT_PATH = candidate_abs
        font_path_var.set(candidate_abs)

def browse_font():
    """点击浏览按钮后，允许用户手动选择字体文件"""
    filename = filedialog.askopenfilename(
        title="选择字体文件",
        filetypes=[("TrueType 字体", "*.ttf"), ("OpenType 字体", "*.otf"), ("所有文件", "*.*")]
    )
    if filename:
        font_path_var.set(filename)
        global FONT_PATH
        FONT_PATH = filename

def on_custom_dimensions(event):
    """当用户手动修改页面大小时，将纸张预设切换为自定义"""
    paper_var.set("自定义")

def on_contour_toggle():
    """当‘轮廓字体’选项状态改变时，控制‘骨架化’选项的启用状态"""
    if not contour_var.get():
        skeleton_var.set(False)
        cb_skeleton.config(state="disabled")
    else:
        cb_skeleton.config(state="normal")

def on_json_export():
    """导出 JSON 按钮事件，调用 on_font_selection 检查字体选择，
    再根据下拉菜单选择的字体获取对应的字体路径后调用生成函数
    """
    # 主动调用 on_font_selection 更新字体路径
    on_font_selection(None)

    try:
        page_width = float(page_width_var.get())
        page_height = float(page_height_var.get())
        top_margin = float(top_margin_var.get())
        bottom_margin = float(bottom_margin_var.get())
        left_margin = float(left_margin_var.get())
        right_margin = float(right_margin_var.get())
        point_size = float(font_size_var.get())
        line_gap_adjust = float(line_gap_adjust_var.get())
    except ValueError:
        messagebox.showerror("输入错误", "请检查数字输入项是否正确！")
        return

    text_content = text_input.get("1.0", tk.END).strip()
    if not text_content:
        messagebox.showerror("输入错误", "请输入文本内容！")
        return

    font_file = get_font_file()
    if not font_file:
        messagebox.showerror("字体错误", "未找到字体文件，请使用浏览按钮选择字体文件！")
        return

    # 根据 UI 中的复选状态获取参数
    allow_closed_paths = contour_var.get()
    bool_skeletonize = skeleton_var.get()

    # 获取 JSON 字符串
    # 获取 JSON 字符串，传入 allow_closed_paths 和 bool_skeletonize 参数
    segment_json_str = prepare_writing_robot_data(
        text_content, font_file, point_size,
        line_gap_adjust=line_gap_adjust,
        allow_closed_paths=allow_closed_paths,
        bool_skeletonize=bool_skeletonize
    )
    segment_data = json.loads(segment_json_str)

    out_data = {
        "page_width": page_width,
        "page_height": page_height,
        "top_margin": top_margin,
        "bottom_margin": bottom_margin,
        "left_margin": left_margin,
        "right_margin": right_margin,
        "segments": [
            segment_data
        ]
    }

    out_json = json.dumps(out_data, ensure_ascii=False, indent=4, sort_keys=False)
    file_path = filedialog.asksaveasfilename(
        title="保存 JSON 文件",
        defaultextension=".json",
        filetypes=[("JSON 文件", "*.json"), ("所有文件", "*.*")]
    )
    if file_path:
        try:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(out_json)
            messagebox.showinfo("保存成功", "JSON 文件已保存至:\n" + file_path)
        except Exception as e:
            messagebox.showerror("保存错误", f"保存 JSON 文件时出错:\n{e}")

# 新增全局变量，标识是否已启动 CDC 监听
cdc_listener_started = False
# 新增全局变量，保存用户选择的 CDC 端口（第一次选择后保持使用）
cdc_selected_port = None

def cdc_data_handler(event_type, data):
    """
    处理来自 CDC 的数据，通过事件类型区分正常数据、错误或端口关闭。
    此处仅打印到控制台，实际应用中可更新界面日志控件等。
    """
    if event_type == "data":
        print("CDC 收到数据:", data)
    elif event_type == "error":
        print("CDC 错误:", data)
    elif event_type == "closed":
        print("CDC 端口关闭:", data)

def on_cdc_send():
    """CDC 输出按钮事件，生成 JSON 数据（压缩成单行），再通过先前选择的 USB CDC 端口发送
       第一次按下时调用 get_available_cdc_ports 让用户选择端口并启动监听，后续直接发送数据
    """
    global cdc_listener_started, cdc_selected_port
    # 先更新字体路径
    on_font_selection(None)
    try:
        page_width = float(page_width_var.get())
        page_height = float(page_height_var.get())
        top_margin = float(top_margin_var.get())
        bottom_margin = float(bottom_margin_var.get())
        left_margin = float(left_margin_var.get())
        right_margin = float(right_margin_var.get())
        point_size = float(font_size_var.get())
        line_gap_adjust = float(line_gap_adjust_var.get())
    except ValueError:
        messagebox.showerror("输入错误", "请检查数字输入项是否正确！")
        return

    text_content = text_input.get("1.0", tk.END).strip()
    if not text_content:
        messagebox.showerror("输入错误", "请输入文本内容！")
        return

    font_file = get_font_file()
    if not font_file:
        messagebox.showerror("字体错误", "未找到字体文件，请使用浏览按钮选择字体文件！")
        return

    # 根据 UI 中的复选状态获取参数
    allow_closed_paths = contour_var.get()
    bool_skeletonize = skeleton_var.get()

    # 跟导出 JSON 的逻辑一致
    segment_json_str = prepare_writing_robot_data(
        text_content, font_file, point_size,
        line_gap_adjust=line_gap_adjust,
        allow_closed_paths=allow_closed_paths,
        bool_skeletonize=bool_skeletonize
    )
    segment_data = json.loads(segment_json_str)
    out_data = {
        "page_width": page_width,
        "page_height": page_height,
        "top_margin": top_margin,
        "bottom_margin": bottom_margin,
        "left_margin": left_margin,
        "right_margin": right_margin,
        "segments": [
            segment_data
        ]
    }
    # 使用 separators 参数压缩为单行 JSON（不添加 indent 和空格）
    out_json = json.dumps(out_data, ensure_ascii=False, separators=(',',':'), sort_keys=False)

    # 第一次使用时，调用 get_available_cdc_ports 让用户选择一个端口
    if not cdc_selected_port:
        ports = get_available_cdc_ports()
        if not ports:
            messagebox.showerror("端口选择", "没有检测到可用的 CDC 端口！")
            return
        selected_port = filedialog.askstring("选择 CDC 端口", "请输入一个可用的CDC端口（例如，" + ", ".join(ports) + "）：")
        if not selected_port:
            messagebox.showerror("端口选择", "未选择 CDC 端口！")
            return
        cdc_selected_port = selected_port
        # 启动监听（仅第一次调用时启动）
        listen_to_cdc(cdc_selected_port, baudrate=9600, data_callback=cdc_data_handler)
        cdc_listener_started = True

    success, msg = send_to_cdc(out_json, cdc_selected_port)
    if success:
        messagebox.showinfo("USB输出", msg)
    else:
        messagebox.showerror("USB输出错误", msg)

def on_exit():
    """退出应用"""
    root.destroy()

root = tk.Tk()
root.title("写字机器人数据生成工具")

# 纸张设置
frame_paper = ttk.LabelFrame(root, text="纸张设置")
frame_paper.grid(row=0, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_paper, text="纸张型号:").grid(row=0, column=0, padx=5, pady=2, sticky="e")
paper_var = tk.StringVar(value="A4")
paper_menu = ttk.OptionMenu(frame_paper, paper_var, "A4", *PAPER_PRESETS.keys(), command=update_margins)
paper_menu.grid(row=0, column=1, padx=5, pady=2, sticky="w")
ttk.Label(frame_paper, text="页面宽度(mm):").grid(row=1, column=0, padx=5, pady=2, sticky="e")
page_width_var = tk.StringVar()
entry_page_width = ttk.Entry(frame_paper, textvariable=page_width_var, width=10)
entry_page_width.grid(row=1, column=1, padx=5, pady=2, sticky="w")
ttk.Label(frame_paper, text="页面高度(mm):").grid(row=2, column=0, padx=5, pady=2, sticky="e")
page_height_var = tk.StringVar()
entry_page_height = ttk.Entry(frame_paper, textvariable=page_height_var, width=10)
entry_page_height.grid(row=2, column=1, padx=5, pady=2, sticky="w")
entry_page_width.bind("<KeyRelease>", on_custom_dimensions)
entry_page_height.bind("<KeyRelease>", on_custom_dimensions)

# 页边距设置
frame_margin = ttk.LabelFrame(root, text="页边距设置")
frame_margin.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_margin, text="上边距(mm):").grid(row=0, column=0, padx=5, pady=2, sticky="e")
top_margin_var = tk.StringVar()
entry_top = ttk.Entry(frame_margin, textvariable=top_margin_var, width=10)
entry_top.grid(row=0, column=1, padx=5, pady=2, sticky="w")
ttk.Label(frame_margin, text="下边距(mm):").grid(row=1, column=0, padx=5, pady=2, sticky="e")
bottom_margin_var = tk.StringVar()
entry_bottom = ttk.Entry(frame_margin, textvariable=bottom_margin_var, width=10)
entry_bottom.grid(row=1, column=1, padx=5, pady=2, sticky="w")
ttk.Label(frame_margin, text="左边距(mm):").grid(row=2, column=0, padx=5, pady=2, sticky="e")
left_margin_var = tk.StringVar()
entry_left = ttk.Entry(frame_margin, textvariable=left_margin_var, width=10)
entry_left.grid(row=2, column=1, padx=5, pady=2, sticky="w")
ttk.Label(frame_margin, text="右边距(mm):").grid(row=3, column=0, padx=5, pady=2, sticky="e")
right_margin_var = tk.StringVar()
entry_right = ttk.Entry(frame_margin, textvariable=right_margin_var, width=10)
entry_right.grid(row=3, column=1, padx=5, pady=2, sticky="w")

# 字体设置
frame_font = ttk.LabelFrame(root, text="字体设置")
frame_font.grid(row=2, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_font, text="字体大小(point):").grid(row=0, column=0, padx=5, pady=2, sticky="e")
font_size_var = tk.StringVar(value="24")
entry_font_size = ttk.Entry(frame_font, textvariable=font_size_var, width=10)
entry_font_size.grid(row=0, column=1, padx=5, pady=2, sticky="w")
line_gap_adjust_var = tk.StringVar(value="10")
ttk.Label(frame_font, text="行距调整(point):").grid(row=1, column=0, padx=5, pady=2, sticky="e")
line_gap_adjust_entry = ttk.Entry(frame_font, textvariable=line_gap_adjust_var, width=10)
line_gap_adjust_entry.grid(row=1, column=1, padx=5, pady=2, sticky="w")

frame_font_path = ttk.LabelFrame(root, text="字体设置")
frame_font_path.grid(row=3, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_font_path, text="字体路径:").grid(row=0, column=0, padx=5, pady=2, sticky="e")
font_path_var = tk.StringVar(value=FONT_PATH)
entry_font_path = ttk.Entry(frame_font_path, textvariable=font_path_var, width=40)
entry_font_path.grid(row=0, column=1, padx=5, pady=2, sticky="w")
btn_browse = ttk.Button(frame_font_path, text="浏览", command=browse_font)
btn_browse.grid(row=0, column=2, padx=5, pady=2)

contour_var = tk.BooleanVar(value=False)   # 轮廓字体，对应 allow_closed_paths
skeleton_var = tk.BooleanVar(value=False)  # 骨架化，对应 bool_skeletonize
# 在 frame_font 建立后添加如下代码（例如在现有字体设置 Entry 之后）：
cb_contour = ttk.Checkbutton(frame_font, text="轮廓字体", variable=contour_var, command=on_contour_toggle)
cb_contour.grid(row=2, column=0, padx=5, pady=2, sticky="w")
cb_skeleton = ttk.Checkbutton(frame_font, text="骨架化（实验性）", variable=skeleton_var)
cb_skeleton.grid(row=2, column=1, padx=5, pady=2, sticky="w")
# 初始时如果未勾选“轮廓字体”，禁用“骨架化”
if not contour_var.get():
    cb_skeleton.config(state="disabled")

# 文本输入区
frame_text = ttk.LabelFrame(root, text="输入文本")
frame_text.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
text_input = tk.Text(frame_text, wrap="word", width=60, height=15)
text_input.pack(fill="both", expand=True, padx=5, pady=5)

# 底部按钮区
# 修改底部按钮区，新按钮“导出JSON”和“使用CDC输出”
frame_buttons = ttk.Frame(root)
frame_buttons.grid(row=5, column=0, padx=10, pady=5, sticky="e")
btn_export = ttk.Button(frame_buttons, text="导出 JSON", command=on_json_export)
btn_export.grid(row=0, column=0, padx=5)
btn_cdc = ttk.Button(frame_buttons, text="使用 CDC 输出", command=on_cdc_send)
btn_cdc.grid(row=0, column=1, padx=5)
btn_exit = ttk.Button(frame_buttons, text="退出", command=on_exit)
btn_exit.grid(row=0, column=2, padx=5)

# 窗口大小自适应
root.grid_rowconfigure(4, weight=1)
root.grid_columnconfigure(0, weight=1)
update_margins()
root.mainloop()