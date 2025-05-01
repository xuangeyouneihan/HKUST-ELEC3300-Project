import tkinter as tk
import tkinter.font as tkfont
from tkinter import ttk, messagebox, filedialog
import yaml
import os
from matplotlib import font_manager
import numpy as np

# 引入已有的 prepare_writing_robot_data 函数
from functions import prepare_writing_robot_data

global root, paper_var, page_width_var, page_height_var, top_margin_var, bottom_margin_var
global left_margin_var, right_margin_var, font_size_var, font_family_var, font_path_var
global text_input, paper_menu, font_select_combo

# 使用 matplotlib 获取系统中所有字体，并构造字体名称到字体路径的映射字典
FONT_DICT = {f.name: f.fname for f in font_manager.fontManager.ttflist}
available_fonts = sorted(FONT_DICT.keys())

# 全局字体文件默认路径（这里默认使用映射字典中的 Arial 字体，如果存在）
FONT_PATH = FONT_DICT.get("Arial", r"C:\Windows\Fonts\arial.ttf")

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
    """获取字体文件路径：
    根据下拉菜单中的字体名称，从 FONT_DICT 获取对应的字体文件路径，
    如果找不到或路径不存在，则返回 font_path_var 中的备用值
    """
    selected_font = font_family_var.get()
    candidate = FONT_DICT.get(selected_font, None)
    if candidate and os.path.exists(candidate):
        return candidate
    fallback_font = font_path_var.get()
    if os.path.exists(fallback_font):
        return fallback_font
    return None

def update_font_weight():
    """
    根据当前选定的字体文件 (font_path_var) 对字重进行更新：
    - 如果文件名中包含“Bold”，认为只有 Bold 这一个字重，设置该选项并将控件禁用；
    - 否则设置为 ["Regular", "Bold"] 供用户选择。
    """
    candidate = font_path_var.get()
    base = os.path.basename(candidate)
    if "Bold" in base:
        font_weight_combo['values'] = ("Bold",)
        font_weight_var.set("Bold")
        font_weight_combo.config(state="disabled")
    else:
        font_weight_combo['values'] = ("Regular", "Bold")
        font_weight_var.set("Regular")
        font_weight_combo.config(state="readonly")

def on_font_selection(event):
    """下拉菜单中选择字体时，根据 FONT_DICT 更新字体文件路径，
    同时更新 font_path_var 和全局变量 FONT_PATH，并调用 update_font_weight 更新字重控件
    """
    selected_font = font_family_var.get()
    candidate = FONT_DICT.get(selected_font, None)
    if candidate and os.path.exists(candidate):
        font_path_var.set(candidate)
        global FONT_PATH
        FONT_PATH = candidate
    update_font_weight()

def browse_font():
    """点击浏览按钮后，允许用户手动选择字体文件"""
    filename = filedialog.askopenfilename(
        title="选择字体文件",
        filetypes=[("TrueType 字体", "*.ttf"), ("OpenType 字体", "*.otf"), ("所有文件", "*.*")]
    )
    if filename:
        font_path_var.set(filename)
        # 浏览时直接将选择的字体文件路径赋给字体名称（用户可自行辨识）
        font_family_var.set(os.path.basename(filename))
        global FONT_PATH
        FONT_PATH = filename

def on_custom_dimensions(event):
    """当用户手动修改页面大小时，将纸张预设切换为自定义"""
    paper_var.set("自定义")

def on_ok():
    """确定按钮事件，调用 on_font_selection 检查字体选择，
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

    segment_yaml_str = prepare_writing_robot_data(text_content, font_file, point_size)
    segment_data = yaml.safe_load(segment_yaml_str)

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

    out_yaml = yaml.dump(out_data, allow_unicode=True, sort_keys=False)
    file_path = filedialog.asksaveasfilename(
        title="保存 YAML 文件",
        defaultextension=".yaml",
        filetypes=[("YAML 文件", "*.yaml"), ("所有文件", "*.*")]
    )
    if file_path:
        try:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(out_yaml)
            messagebox.showinfo("保存成功", "YAML 文件已保存至:\n" + file_path)
        except Exception as e:
            messagebox.showerror("保存错误", f"保存 YAML 文件时出错:\n{e}")

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
font_size_var = tk.StringVar(value="12")
entry_font_size = ttk.Entry(frame_font, textvariable=font_size_var, width=10)
entry_font_size.grid(row=0, column=1, padx=5, pady=2, sticky="w")

# 字体选择控件（仅显示字体名称，下拉框的数据来源于 matplotlib 的 FONT_DICT）
frame_font_select = ttk.LabelFrame(root, text="字体选择")
frame_font_select.grid(row=3, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_font_select, text="字体:").grid(row=0, column=0, padx=5, pady=2, sticky="e")
font_family_var = tk.StringVar(value="Arial")
font_path_var = tk.StringVar(value=FONT_PATH)
font_select_combo = ttk.Combobox(frame_font_select, textvariable=font_family_var,
                                values=available_fonts, state="readonly", width=30)
font_select_combo.grid(row=0, column=1, padx=5, pady=2, sticky="w")
font_select_combo.bind("<<ComboboxSelected>>", on_font_selection)
btn_browse = ttk.Button(frame_font_select, text="浏览", command=browse_font)
btn_browse.grid(row=0, column=2, padx=5, pady=2)

# 新增字重选择控件
ttk.Label(frame_font_select, text="字重:").grid(row=1, column=0, padx=5, pady=2, sticky="e")
font_weight_var = tk.StringVar(value="Regular")
font_weight_combo = ttk.Combobox(frame_font_select, textvariable=font_weight_var,
                                state="readonly", width=30)
font_weight_combo.grid(row=1, column=1, padx=5, pady=2, sticky="w")

# 文本输入区
frame_text = ttk.LabelFrame(root, text="输入文本")
frame_text.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
text_input = tk.Text(frame_text, wrap="word", width=60, height=15)
text_input.pack(fill="both", expand=True, padx=5, pady=5)

# 底部按钮区
frame_buttons = ttk.Frame(root)
frame_buttons.grid(row=5, column=0, padx=10, pady=5, sticky="e")
btn_ok = ttk.Button(frame_buttons, text="确定", command=on_ok)
btn_ok.grid(row=0, column=0, padx=5)
btn_exit = ttk.Button(frame_buttons, text="退出", command=on_exit)
btn_exit.grid(row=0, column=1, padx=5)

# 窗口大小自适应
root.grid_rowconfigure(4, weight=1)
root.grid_columnconfigure(0, weight=1)
update_margins()
root.mainloop()