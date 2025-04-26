from fontTools.pens.boundsPen import BoundsPen
from fontTools.pens.recordingPen import RecordingPen
import yaml
import unicodedata
from fontTools.ttLib import TTFont


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

def get_line_gap(font, scale=1.0):
    """
    获取字体的 lineGap（行间距），乘以 scale 后返回（单位为 point）。
    """
    if "hhea" in font:
        return font["hhea"].lineGap * scale
    elif "OS/2" in font:
        return font["OS/2"].sTypoLineGap * scale
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

def get_line_gap_in_mm(font, point_size):
    """
    给定字体和目标字号，返回 lineGap（行间距），单位：毫米。
    """
    upm = font["head"].unitsPerEm
    scale = point_size / upm
    line_gap_pt = get_line_gap(font, scale)
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

class StrokePen(object):
    """
    自定义笔，用于记录字形各笔画上的坐标点，按绘制顺序排列。
    坐标记录直接基于字体设计坐标（基线 y=0 为原点），不做平移处理。
    """
    def __init__(self, scale=1.0):
        self.scale = scale
        self.strokes = []
        self.current_stroke = []
    
    def _apply_scale(self, pt):
        return (pt[0] * self.scale, pt[1] * self.scale)
    
    def moveTo(self, pt):
        if self.current_stroke:
            self.strokes.append(self.current_stroke)
        self.current_stroke = [self._apply_scale(pt)]
    
    def lineTo(self, pt):
        self.current_stroke.append(self._apply_scale(pt))
    
    def curveTo(self, *points):
        for pt in points:
            self.current_stroke.append(self._apply_scale(pt))
    
    def qCurveTo(self, *points):
        for pt in points:
            self.current_stroke.append(self._apply_scale(pt))
    
    def closePath(self):
        if self.current_stroke:
            self.strokes.append(self.current_stroke)
            self.current_stroke = []
    
    def endPath(self):
        if self.current_stroke:
            self.strokes.append(self.current_stroke)
            self.current_stroke = []

def get_stroke_points_in_mm(font, char, point_size):
    """
    根据字体、字符和目标字号（point），提取该字符所有笔画上的坐标点（单位：毫米）。
    坐标点按笔顺排列，直接以字体坐标（基线 y=0 为原点）为准，不做额外平移。
    支持汉字。
    """
    # 计算缩放因子：设计单位（upm）转换到目标 point 尺寸下
    upm = font["head"].unitsPerEm
    scale = point_size / upm

    # 获取字符对应的 glyph 名称
    cmap = font.getBestCmap()
    code = ord(char)
    if code not in cmap:
        return None
    glyph_name = cmap[code]
    
    pen = StrokePen(scale=scale)
    
    if "glyf" in font:
        glyph_obj = font["glyf"][glyph_name]
        glyph_obj.draw(pen)
    elif "CFF " in font:
        try:
            cff_table = font["CFF "].cff
            top_dict = cff_table.topDictIndex[0]
            charString = top_dict.CharStrings[glyph_name]
            charString.draw(pen)
        except Exception:
            return None
    else:
        return None

    if pen.current_stroke:
        pen.strokes.append(pen.current_stroke)

    # 将每个点转换为毫米单位，同时使用列表而不是元组，避免 YAML 中出现 !!python/tuple
    strokes = []
    for stroke in pen.strokes:
        stroke_points = []
        for (x, y) in stroke:
            stroke_points.append([pt_to_mm(x), pt_to_mm(y)])
        strokes.append(stroke_points)
    
    return strokes

def prepare_writing_robot_data(text, font_path, point_size):
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
    font = TTFont(font_path)
    
    # 提取全局行高相关参数（单位：毫米）
    ascender = get_ascender_in_mm(font, point_size)
    descender = get_descender_in_mm(font, point_size)
    line_gap = get_line_gap_in_mm(font, point_size)
    
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
    
    return yaml.dump(data, allow_unicode=True, sort_keys=False)