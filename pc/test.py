import tkinter as tk
from tkinter import filedialog, messagebox
import yaml
import turtle

def draw_strokes(segment):
    # 清空 turtle 画布
    screen = turtle.Screen()
    screen.clearscreen()
    t = turtle.Turtle()
    t.hideturtle()
    t.speed(0)  # 最快绘制

    # 简单设置缩放比例（若坐标单位为毫米，可以酌情调整）
    scale = 50

    # 遍历 segment 中每个字符的笔画
    characters = segment.get("characters", [])
    for char_item in characters:
        strokes = char_item.get("strokes", [])
        for stroke in strokes:
            if stroke:
                # 将第一个点作为起点
                x0, y0 = stroke[0]
                t.penup()
                t.goto(x0 * scale, y0 * scale)
                t.pendown()
                # 依次绘制笔画中的后续坐标点
                for pt in stroke[1:]:
                    x, y = pt
                    t.goto(x * scale, y * scale)
                t.penup()  # 一个笔画绘制完毕

    # 绘图完成后启动计时器，3秒后关闭窗口
    screen.ontimer(screen.bye, 3000)
    screen.mainloop()

def browse_yaml():
    file_path = filedialog.askopenfilename(
        title="选择 YAML 文件",
        filetypes=[("YAML 文件", "*.yaml"), ("所有文件", "*.*")]
    )
    if file_path:
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = yaml.safe_load(f)
            # 取 segments 列表的第 1 项（注意此处假定 segments 存在且不为空）
            segments = data.get("segments", [])
            if not segments:
                messagebox.showerror("错误", "YAML 文件中没有 segments 数据!")
                return
            segment = segments[0]
            draw_strokes(segment)
        except Exception as e:
            messagebox.showerror("错误", f"读取或绘制 YAML 文件时出错:\n{e}")

def main():
    root = tk.Tk()
    root.title("YAML 字体绘制测试程序")
    root.geometry("300x100")

    btn_browse = tk.Button(root, text="浏览 YAML 文件", command=browse_yaml)
    btn_browse.pack(expand=True)

    root.mainloop()

if __name__ == "__main__":
    main()