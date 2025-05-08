from fontTools.pens.boundsPen import BoundsPen
from fontTools.pens.recordingPen import RecordingPen
from fontTools.pens.transformPen import TransformPen
import json
import unicodedata
from fontTools.ttLib import TTFont, TTCollection
import os

def pt_to_mm(points):
    """
    将 point 单位转换为毫米：(25.4mm / 72pt) ≈ 0.3528 mm/pt。
    """
    return points * 25.4 / 72

def get_ascender(font, scale=1.0):
    """
    获取字体的 ascender（上升高度），乘以 scale 后返回（单位为 point）。
    """
    if "hhea" in font:
        return font["hhea"].ascender * scale
    elif "OS/2" in font:
        return font["OS/2"].sTypoAscender * scale
    else:
        return None

def get_descender(font, scale=1.0):
    """
    获取字体的 descender（下降高度，一般为负值），乘以 scale 后返回（单位为 point）。
    """
    if "hhea" in font:
        return font["hhea"].descender * scale
    elif "OS/2" in font:
        return font["OS/2"].sTypoDescender * scale
    else:
        return None

def get_line_gap(font, scale=1.0, adjust=0):
    """
    获取字体的 lineGap（行间距），加上用户指定的 adjust 后再乘以 scale（单位为 point），返回最终的行间距。
    """
    if "hhea" in font:
        return (font["hhea"].lineGap + adjust) * scale
    elif "OS/2" in font:
        return (font["OS/2"].sTypoLineGap + adjust) * scale
    else:
        return None

def get_line_height(font, scale=1.0):
    """
    获取字体的行高（单位为 point），计算公式：(ascender - descender) + lineGap。
    """
    ascender = get_ascender(font, scale)
    descender = get_descender(font, scale)
    line_gap = get_line_gap(font, scale)
    if ascender is None or descender is None or line_gap is None:
        return None
    return (ascender - descender) + line_gap

def get_ascender_in_mm(font, point_size):
    """
    给定字体和目标字号，返回 ascender（上升高度），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    ascender_pt = get_ascender(font, scale)
    if ascender_pt is None:
        return None
    return pt_to_mm(ascender_pt)

def get_descender_in_mm(font, point_size):
    """
    给定字体和目标字号，返回 descender（下降高度），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    descender_pt = get_descender(font, scale)
    if descender_pt is None:
        return None
    return pt_to_mm(descender_pt)

def get_line_gap_in_mm(font, point_size, adjust=0):
    """
    给定字体和目标字号，返回 lineGap（行间距），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    line_gap_pt = get_line_gap(font, scale, adjust)
    if line_gap_pt is None:
        return None
    return pt_to_mm(line_gap_pt)

def get_line_height_in_mm(font, point_size):
    """
    给定字体和目标字号，返回行高（单位：毫米）。
    行高公式：(ascender - descender) + lineGap，经转换后单位为毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    line_height_pt = get_line_height(font, scale)
    if line_height_pt is None:
        return None
    return pt_to_mm(line_height_pt)

def get_advance_width(font, char, scale=1.0):
    """
    根据单个字符和字体文件，在 hmtx 表中获取该字符的 advance width。
    """
    cmap = font.getBestCmap()
    code = ord(char)
    if code not in cmap:
        return None
    glyph_name = cmap[code]
    if "hmtx" in font:
        try:
            advance_width, lsb = font["hmtx"].metrics[glyph_name]
            return advance_width * scale
        except KeyError:
            return None
    else:
        return None

def get_left_side_bearing(font, char, scale=1.0):
    """
    根据单个字符和字体文件，在 hmtx 表中获取该字符的 left side bearing（左侧间距）。
    返回值乘以 scale 后单位为 point。
    """
    cmap = font.getBestCmap()
    code = ord(char)
    if code not in cmap:
        return None
    glyph_name = cmap[code]
    if "hmtx" not in font:
        return None
    try:
        # hmtx.metrics 返回 (advance width, left side bearing)
        _, lsb = font["hmtx"].metrics[glyph_name]
        return lsb * scale
    except KeyError:
        return None

def get_glyph_width(font, char, scale=1.0):
    """
    根据单个字符和字体文件，计算该字形的宽度（glyph width）。
    对 TrueType 字体优先使用 glyf 表，若不存在则尝试使用 CFF 表。
    返回值乘以 scale 后单位为 point。
    """
    cmap = font.getBestCmap()
    code = ord(char)
    if code not in cmap:
        return None
    glyph_name = cmap[code]
    
    # 优先使用 glyf 表
    if "glyf" in font:
        glyf_table = font["glyf"]
        glyph_obj = glyf_table[glyph_name]
        # 对简单字形，直接使用 xMin/xMax；组合字形则使用 RecordingPen记录坐标计算范围
        if not glyph_obj.isComposite():
            if glyph_obj.xMin is not None and glyph_obj.xMax is not None:
                return (glyph_obj.xMax - glyph_obj.xMin) * scale
            else:
                return None
        else:
            pen = RecordingPen()
            glyph_obj.draw(pen)
            xs = []
            for command, points in pen.value:
                for pt in points:
                    xs.append(pt[0])
            if xs:
                minX, maxX = min(xs), max(xs)
                return (maxX - minX) * scale
            else:
                return None
    # 若无 glyf 表则尝试使用 CFF 表
    elif "CFF " in font:
        try:
            cff_table = font["CFF "].cff
            top_dict = cff_table.topDictIndex[0]
            charString = top_dict.CharStrings[glyph_name]
        except Exception:
            return None
        pen = BoundsPen(None)
        try:
            charString.draw(pen)
        except Exception:
            return None
        if pen.bounds is None:
            return None
        minX, _, maxX, _ = pen.bounds
        return (maxX - minX) * scale
    else:
        return None

def get_advance_width_in_mm(font, char, point_size):
    """
    给定单个字符、字体和目标字号，返回字符的 advance width（单位：毫米）。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    advance_width_pt = get_advance_width(font, char, scale)
    if advance_width_pt is None:
        return None
    return pt_to_mm(advance_width_pt)

def get_left_side_bearing_in_mm(font, char, point_size):
    """
    给定字体、字符和目标字号，返回该字符的 left side bearing（左侧间距），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    lsb_pt = get_left_side_bearing(font, char, scale)
    if lsb_pt is None:
        return None
    return pt_to_mm(lsb_pt)

def get_glyph_width_in_mm(font, char, point_size):
    """
    给定字体、字符和目标字号，返回该字符的 glyph width（字形宽度），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    width_pt = get_glyph_width(font, char, scale)
    if width_pt is None:
        return None
    return pt_to_mm(width_pt)

class MyTransformPen(TransformPen):
    def _transformPoint(self, pt):
        # 确保 pt 是一个 (float, float) 的元组
        x, y = pt
        return super()._transformPoint((float(x), float(y)))

def flatten_transform(transform):
    """
    递归将 transform 列表扁平化，确保所有元素均为数值。
    """
    flattened = []
    for v in transform:
        if isinstance(v, (list, tuple)):
            flattened.extend(flatten_transform(v))
        else:
            flattened.append(v)
    return tuple(flattened)

def get_stroke_points_in_mm(font, char, point_size):
    """
    使用 RecordingPen 记录字形绘制路径，支持组件（composite glyph）功能，
    将记录的所有点按设计单位先乘以缩放因子，再经 pt_to_mm 转换为毫米单位，
    最后返回每笔画的点序列列表。
    对于所有字形（包括复合字形），遇到 closePath 命令时均不闭合路径，而是直接结束当前笔画。
    """
    # 计算缩放因子，从字体设计单位转换到目标 point 单位
    upm = font["head"].unitsPerEm
    scale = point_size / upm

    cmap = font.getBestCmap()
    code = ord(char)
    if code not in cmap:
        return None
    glyph_name = cmap[code]

    pen = RecordingPen()
    if "glyf" in font:
        glyph = font["glyf"][glyph_name]
        if glyph.isComposite():
            # 对于复合字形，遍历每个组件
            for comp in glyph.components:
                compGlyph = font["glyf"][comp.glyphName]

                transform_from_attr = None
                translation = None

                # 如果有 transform 属性，则提取
                if hasattr(comp, 'transform'):
                    flat_tr = flatten_transform(comp.transform)
                    transform_from_attr = tuple(float(v) for v in flat_tr)
                    # 如果仅返回4个值，则补充 (0,0)
                    if len(transform_from_attr) == 4:
                        transform_from_attr = transform_from_attr + (0.0, 0.0)
                
                # 如果有 x 和 y 属性，则构造平移矩阵
                if hasattr(comp, 'x') and hasattr(comp, 'y'):
                    translation = (1, 0, 0, 1, float(comp.x), float(comp.y))
                
                # 合并两种变换：
                # 如果两者都有，则先应用 transform 属性，再加上 x/y 平移
                if transform_from_attr is not None and translation is not None:
                    a, b, c, d, e, f = transform_from_attr
                    _, _, _, _, tx, ty = translation
                    combined = (a, b, c, d, e + tx, f + ty)
                elif transform_from_attr is not None:
                    combined = transform_from_attr
                elif translation is not None:
                    combined = translation
                else:
                    combined = (1, 0, 0, 1, 0, 0)

                tPen = MyTransformPen(pen, combined)
                compGlyph.draw(tPen, font["glyf"])
        else:
            glyph.draw(pen, font["glyf"])
    elif "CFF " in font:
        try:
            cff_table = font["CFF "].cff
            top_dict = cff_table.topDictIndex[0]
            charString = top_dict.CharStrings[glyph_name]
            charString.draw(pen)
        except Exception:
            return None

    # 解析 RecordingPen 记录的绘制命令，提取各笔画的点序列
    strokes = []
    current_stroke = []
    for command, points in pen.value:
        if command == "moveTo":
            if current_stroke:
                strokes.append(current_stroke)
            # 新笔画的起点
            current_stroke = [points[0]]
        elif command in ("lineTo", "curveTo", "qCurveTo"):
            if not current_stroke:
                current_stroke = []
            current_stroke.extend(points)
        elif command == "closePath":
            if current_stroke:
                # 对于所有字形直接结束当前笔画，不闭合路径
                strokes.append(current_stroke)
                current_stroke = []
    if current_stroke:
        strokes.append(current_stroke)

    # 将每个点转换为毫米单位：先乘以 scale (转换为 point 单位)，再转换为毫米
    new_strokes = []
    for stroke in strokes:
        new_stroke = []
        for (x, y) in stroke:
            new_stroke.append([pt_to_mm(x * scale), pt_to_mm(y * scale)])
        new_strokes.append(new_stroke)
    return new_strokes

def load_font(font_path, font_number=0):
    """
    根据字体文件路径加载字体。
    如果是 TTC 或 OTC 字体集合，则返回集合中的第 font_number 个字体。
    否则直接加载返回 TTFont 对象。
    """
    ext = os.path.splitext(font_path)[1].lower()
    if ext in [".ttc", ".otc"]:
        collection = TTCollection(font_path)
        return collection.fonts[font_number]
    else:
        return TTFont(font_path)

def prepare_writing_robot_data(text, font_path, point_size, line_gap_adjust):
    """
    根据输入字符串、字体文件路径和目标字号（point），生成写字机器人数据，
    数据包含如下键：
    
    ascender: <ascender值>
    descender: <descender值>
    line_gap: <line_gap值>
    paragraph_spacing: 0
    characters:
      -   # 第1个字符
            is_line_feed: False
            advance_width: <advance_width（毫米）>
            left_side_bearing: <left_side_bearing（毫米）>
            strokes:
              - [[x1, y1], [x2, y2], ...]   # 第1笔
              - [[x3, y3], [x4, y4], ...]   # 第2笔
      -  # 第2个字符
         ...
    
    修改增加：若字符为制表符 ("\t")，检查其后字符，如为全角字符则其 advance_width = 2 * (全角空格宽度)，否则 = 4 * (半角空格宽度)。
    均以毫米为单位，坐标直接以基线 y=0 为原点。
    """
    # 加载字体文件
    font = load_font(font_path)
    
    # 提取全局行高相关参数（单位：毫米）
    ascender = get_ascender_in_mm(font, point_size)
    descender = get_descender_in_mm(font, point_size)
    line_gap = get_line_gap_in_mm(font, point_size, adjust=line_gap_adjust)
    
    # 构造顶层数据结构（移除 segments 键）
    data = {
        "ascender": ascender if ascender is not None else 0,
        "descender": descender if descender is not None else 0,
        "line_gap": line_gap if line_gap is not None else 0,
        "paragraph_spacing": 0,
        "characters": []
    }
    
    # 预先获取全角空格和半角空格的宽度
    fullwidth_space = '　'  # 全角空格 (U+3000)
    halfwidth_space = ' '   # 半角空格
    fw_space_width = get_advance_width_in_mm(font, fullwidth_space, point_size) or 0
    hw_space_width = get_advance_width_in_mm(font, halfwidth_space, point_size) or 0
    
    for i, ch in enumerate(text):
        if ch == "\n":
            # 换行符处理
            char_item = {
                "is_line_feed": True,
                "advance_width": 0,
                "left_side_bearing": 0,
                "strokes": []
            }
        elif ch == "\t":
            # 制表符处理, 检查其后字符是否为全角
            if (i + 1) < len(text) and unicodedata.east_asian_width(text[i+1]) in ['F', 'W']:
                adv = 2 * fw_space_width
            else:
                adv = 4 * hw_space_width
            char_item = {
                "is_line_feed": False,
                "advance_width": adv,
                "left_side_bearing": 0,
                "strokes": []
            }
        elif ch == " " or ch == "　":
            # 如果字符为空格，则不解析笔画
            adv = get_advance_width_in_mm(font, ch, point_size)
            lsb = get_left_side_bearing_in_mm(font, ch, point_size)
            char_item = {
                "is_line_feed": False,
                "advance_width": adv if adv is not None else 0,
                "left_side_bearing": lsb if lsb is not None else 0,
                "strokes": []
            }
        else:
            adv = get_advance_width_in_mm(font, ch, point_size)
            lsb = get_left_side_bearing_in_mm(font, ch, point_size)
            strokes = get_stroke_points_in_mm(font, ch, point_size)
            if strokes is None:
                strokes = []
            char_item = {
                "is_line_feed": False,
                "advance_width": adv if adv is not None else 0,
                "left_side_bearing": lsb if lsb is not None else 0,
                "strokes": strokes
            }
        data["characters"].append(char_item)
    
    return json.dumps(data, ensure_ascii=False, indent=4, sort_keys=False)