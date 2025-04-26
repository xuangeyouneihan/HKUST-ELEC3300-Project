import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext, filedialog
import yaml
import os

# 引入已有的prepare_writing_robot_data函数及依赖函数
from functions import prepare_writing_robot_data

# 字体文件路径（示例使用 Windows 系统的 Arial 字体，请根据需要修改）
FONT_PATH = r"C:\Windows\Fonts\arial.ttf"

# 定义纸张尺寸（单位：毫米）及对应默认页边距（单位：毫米），这里仅给出示例数据
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
    preset = paper_var.get()
    if preset in PAPER_PRESETS:
        config = PAPER_PRESETS[preset]
        top_margin_var.set(str(config["top_margin"]))
        bottom_margin_var.set(str(config["bottom_margin"]))
        left_margin_var.set(str(config["left_margin"]))
        right_margin_var.set(str(config["right_margin"]))
        page_width_var.set(str(config["page_width"]))
        page_height_var.set(str(config["page_height"]))

def on_ok():
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

    # 使用用户选定的字体文件
    font_file = font_path_var.get()

    # 调用已有函数生成字符数据（YAML字符串），并解析为字典
    segment_yaml_str = prepare_writing_robot_data(text_content, font_file, point_size)
    segment_data = yaml.safe_load(segment_yaml_str)

    # 构造最终的 YAML 格式数据
    out_data = {
        "page_width": page_width,
        "page_height": page_height,
        "top_margin": top_margin,
        "bottom_margin": bottom_margin,
        "left_margin": left_margin,
        "right_margin": right_margin,
        "segments": [
            segment_data  # 将 prepare_writing_robot_data 的内容放在 segments 的第 1 项中
        ]
    }

    out_yaml = yaml.dump(out_data, allow_unicode=True, sort_keys=False)

    # 弹出文件保存对话框，保存生成的 YAML 文件
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
    root.destroy()

root = tk.Tk()
root.title("写字机器人数据生成工具")

# 设置纸张尺寸预设
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

# 当用户手动输入页面大小时，自动将纸张型号切换为“自定义”
def on_custom_dimensions(event):
    paper_var.set("自定义")

entry_page_width.bind("<KeyRelease>", on_custom_dimensions)
entry_page_height.bind("<KeyRelease>", on_custom_dimensions)

# 设置页边距
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

# 字体设置（单位：point）
frame_font = ttk.LabelFrame(root, text="字体设置")
frame_font.grid(row=2, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_font, text="字体大小(point):").grid(row=0, column=0, padx=5, pady=2, sticky="e")
font_size_var = tk.StringVar(value="12")
entry_font_size = ttk.Entry(frame_font, textvariable=font_size_var, width=10)
entry_font_size.grid(row=0, column=1, padx=5, pady=2, sticky="w")

# 新增字体选择控件
frame_font_select = ttk.LabelFrame(root, text="字体选择")
frame_font_select.grid(row=3, column=0, padx=10, pady=5, sticky="ew")
ttk.Label(frame_font_select, text="字体文件路径:").grid(row=0, column=0, padx=5, pady=2, sticky="e")
font_path_var = tk.StringVar(value=FONT_PATH)
entry_font_path = ttk.Entry(frame_font_select, textvariable=font_path_var, width=50)
entry_font_path.grid(row=0, column=1, padx=5, pady=2, sticky="w")
def browse_font():
    filename = filedialog.askopenfilename(
        title="选择字体文件",
        filetypes=[("TrueType 字体", "*.ttf"), ("OpenType 字体", "*.otf"), ("所有文件", "*.*")]
    )
    if filename:
        font_path_var.set(filename)
btn_browse = ttk.Button(frame_font_select, text="浏览", command=browse_font)
btn_browse.grid(row=0, column=2, padx=5, pady=2)

# 文本输入框
frame_text = ttk.LabelFrame(root, text="输入文本")
frame_text.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
text_input = tk.Text(frame_text, wrap="word", width=60, height=15)
text_input.pack(fill="both", expand=True, padx=5, pady=5)

# 底部按钮
frame_buttons = ttk.Frame(root)
frame_buttons.grid(row=5, column=0, padx=10, pady=5, sticky="e")
btn_ok = ttk.Button(frame_buttons, text="确定", command=on_ok)
btn_ok.grid(row=0, column=0, padx=5)
btn_exit = ttk.Button(frame_buttons, text="退出", command=on_exit)
btn_exit.grid(row=0, column=1, padx=5)

# 调整窗口大小自适应
root.grid_rowconfigure(4, weight=1)
root.grid_columnconfigure(0, weight=1)

# 初始化预设纸张数据
update_margins()

root.mainloop()