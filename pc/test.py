import tkinter as tk
from tkinter import filedialog, messagebox
import json
import turtle

def draw_document(document):
    """
    根据 document 中的页面参数设置画布，并从左上角 (left_margin, top_margin) 开始依次绘制各段内容。
    坐标单位为毫米，通过 scale 换算为 turtle 的坐标单位
    """
    scale = 2.5
    page_width = document.get("page_width", 210)
    page_height = document.get("page_height", 297)
    left_margin = document.get("left_margin", 25.4)
    top_margin = document.get("top_margin", 25.4)
    # 设置画布大小，利用 setworldcoordinates 定义左上角为 (0,0)
    scr = turtle.Screen()
    # 设置画布区域为整个页面尺寸（乘以 scale），注意 turtle 的坐标系统设置为：左上角 (0,0)，右下角 (page_width*scale, page_height*scale)
    scr.setup(width=page_width*scale+50, height=page_height*scale+50)
    scr.setworldcoordinates(0, page_height*scale, page_width*scale, 0)
    
    # 初始化全局起始位置：从左上角偏移 left_margin 和 top_margin（单位：mm）
    Global_X = left_margin
    Global_Y = top_margin
    segments = document.get("segments", [])
    for segment in segments:
        # 忽略空段
        if not segment.get("characters"):
            continue
        Global_X, Global_Y = draw_segment(segment, Global_X, Global_Y, document, scale)
    
    turtle.done()

def draw_segment(segment, Global_X, Global_Y, document, scale):
    """
    根据 C 代码逻辑绘制一个段落（segment）。
    初始位置为 (Global_X, Global_Y)，返回新的全局位置。
    示例中：
      - current_Y 从 Global_Y 加上 segment 的 ascender 开始绘制
      - 检查若 current_X 超过 (page_width - right_margin) 或字符为换行，则换行，
        换行时：current_X 重置为 left_margin，current_Y 下移（增大）相应行高
    """
    left_margin = document.get("left_margin", 25.4)
    right_margin = document.get("right_margin", 25.4)
    page_width = document.get("page_width", 210)
    
    ascender = segment.get("ascender", 0)
    descender = segment.get("descender", 0)
    line_gap = segment.get("line_gap", 0)
    
    current_X = Global_X
    # 注意：由于设置坐标为左上角为起点，向下为 Y 增大，因此行初始 Y 位置为 Global_Y + ascender
    current_Y = Global_Y + ascender

    for character in segment.get("characters", []):
        advance_width = character.get("advance_width", 0)
        is_line_feed = character.get("is_line_feed", False)
        # 判断是否需要换行：超出右边距或者字符为换行符
        if (current_X + advance_width) >= (page_width - right_margin) or is_line_feed:
            current_X = left_margin
            # 换行后，下移行高（这里采用 ascender + descender + line_gap，单位 mm）
            current_Y += (ascender + descender + line_gap)
            # 遇到换行，如为换行符时，不绘制字符本身
            if is_line_feed:
                continue
        draw_character(character, current_X, current_Y, scale)
        current_X += advance_width

    # 更新全局 Y：回退 ascender，即当前行的基线
    Global_Y = current_Y - ascender
    return current_X, Global_Y

def draw_character(character, startX, startY, scale):
    """
    绘制单个字符。
    字符的左起始位置为 startX，加上 left_side_bearing 得到字符实际起始位置，
    然后绘制每一笔画 (stroke)。
    """
    left_side_bearing = character.get("left_side_bearing", 0)
    charStartX = startX + left_side_bearing
    for stroke in character.get("strokes", []):
        if not stroke:
            continue
        draw_stroke(stroke, charStartX, startY, scale)

def draw_stroke(stroke, offsetX, offsetY, scale):
    """
    绘制一个笔画。所有点的坐标为相对值（单位：毫米）。
    绘制前将每个点加上偏移量，并乘以缩放比例 scale。
    修改后：将 y 方向的相加改为相减，以纠正旋转180°的问题，但保持字符整体位置不变。
    """
    t = turtle.Turtle()
    t.hideturtle()
    t.speed(0)
    t.penup()
    # 移动到第一个点（将 y 坐标从 offsetY + y 改为 offsetY - y）
    x0, y0 = stroke[0]
    t.goto((offsetX + x0) * scale, (offsetY - y0) * scale)
    t.pendown()
    for pt in stroke[1:]:
        x, y = pt
        t.goto((offsetX + x) * scale, (offsetY - y) * scale)
    t.penup()
    t.hideturtle()

def browse_json():
    file_path = filedialog.askopenfilename(
        title="选择 JSON 文件",
        filetypes=[("JSON 文件", "*.json"), ("所有文件", "*.*")]
    )
    if file_path:
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                document = json.load(f)
            draw_document(document)
        except Exception as e:
            messagebox.showerror("错误", f"读取或绘制 JSON 文件时出错:\n{e}")

def main():
    root = tk.Tk()
    root.title("JSON 文档绘制测试程序")
    root.geometry("300x100")

    btn_browse = tk.Button(root, text="浏览 JSON 文件", command=browse_json)
    btn_browse.pack(expand=True)

    root.mainloop()

if __name__ == "__main__":
    main()