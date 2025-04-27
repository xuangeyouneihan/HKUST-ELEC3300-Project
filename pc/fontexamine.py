import sys
import os
from fontTools.ttLib import TTFont, TTCollection
from fontTools.pens.recordingPen import RecordingPen
import pprint

def load_font(font_path, font_number=0):
    """
    根据字体文件路径加载字体。
    如果是 TTC 或 OTC 字体集合，则返回集合中的第 font_number 个字体；
    否则直接加载返回 TTFont 对象。
    """
    ext = os.path.splitext(font_path)[1].lower()
    if ext in [".ttc", ".otc"]:
        collection = TTCollection(font_path)
        return collection.fonts[font_number]
    else:
        return TTFont(font_path)

def print_global_info(font):
    print("----- Global Font Information -----")
    if "head" in font:
        head = font["head"]
        print("Table 'head':")
        print("  unitsPerEm: ", head.unitsPerEm)
        print("  xMin: ", head.xMin, ", xMax: ", head.xMax)
        print("  yMin: ", head.yMin, ", yMax: ", head.yMax)
    if "hhea" in font:
        hhea = font["hhea"]
        print("Table 'hhea':")
        print("  ascender: ", hhea.ascender)
        print("  descender: ", hhea.descender)
        print("  lineGap: ", hhea.lineGap)
    if "OS/2" in font:
        os2 = font["OS/2"]
        print("Table 'OS/2':")
        print("  sTypoAscender: ", os2.sTypoAscender)
        print("  sTypoDescender: ", os2.sTypoDescender)
        print("  sTypoLineGap: ", os2.sTypoLineGap)
    print("-----------------------------------")

def print_gidx_info(font, gid):
    print("----- Glyph ID {} Information -----".format(gid))
    glyph_order = font.getGlyphOrder()
    if len(glyph_order) <= gid:
        print("该字体没有 glyph id {}。".format(gid))
        return
    # 获取指定 gid 所对应的 glyph 名称
    glyph_name = glyph_order[gid]
    print("Glyph ID {} 名称: {}".format(gid, glyph_name))
    
    # 查找该 glyph 对应的 Unicode 字符
    cmap = font.getBestCmap()
    matched_codes = [code for code, name in cmap.items() if name == glyph_name]
    if matched_codes:
        for code in matched_codes:
            print("对应字符: {} (U+{:04X})".format(chr(code), code))
    else:
        print("未找到对应字符。")
    
    # 针对 TrueType 字体 (glyf 表) 的情况：
    if "glyf" in font:
        glyf = font["glyf"]
        glyph = glyf[glyph_name]
        print("Glyph 对象:", glyph)
        if not glyph.isComposite():
            print("简单字形指标:")
            head_xmin = font["head"].xMin if "head" in font else None
            head_xmax = font["head"].xMax if "head" in font else None
            head_ymin = font["head"].yMin if "head" in font else None
            head_ymax = font["head"].yMax if "head" in font else None

            # 使用 getattr 获取，如不存在则采用 head 表的值
            xmin = getattr(glyph, 'xMin', head_xmin)
            xmax = getattr(glyph, 'xMax', head_xmax)
            ymin = getattr(glyph, 'yMin', head_ymin)
            ymax = getattr(glyph, 'yMax', head_ymax)
            
            print("  xMin:", xmin, "  xMax:", xmax)
            print("  yMin:", ymin, "  yMax:", ymax)
        else:
            print("复合字形。")
        # 获取绘制指令
        from fontTools.pens.recordingPen import RecordingPen
        pen = RecordingPen()
        try:
            glyph.draw(pen, glyf)
            print("绘制指令:")
            import pprint
            pprint.pprint(pen.value)
        except Exception as e:
            print("绘制字形时出错：", e)
        # 输出 glyph 对象本身详细信息
        print("Glyph 对象详细信息:")
        try:
            import pprint
            pprint.pprint(glyph.__dict__)
        except Exception as e:
            print("无法获取详细属性：", e)
    
    # 针对 CFF 字体的情况：
    elif "CFF " in font:
        try:
            cff_table = font["CFF "].cff
            top_dict = cff_table.topDictIndex[0]
            charString = top_dict.CharStrings[glyph_name]
            print("CFF 字形对象:", charString)
            try:
                bounds = charString.calcBounds()
                if bounds:
                    xmin, ymin, xmax, ymax = bounds
                else:
                    xmin = ymin = xmax = ymax = None
            except Exception:
                xmin = ymin = xmax = ymax = None

            head_xmin = font["head"].xMin if "head" in font else None
            head_xmax = font["head"].xMax if "head" in font else None
            head_ymin = font["head"].yMin if "head" in font else None
            head_ymax = font["head"].yMax if "head" in font else None

            xmin = xmin if xmin is not None else head_xmin
            xmax = xmax if xmax is not None else head_xmax
            ymin = ymin if ymin is not None else head_ymin
            ymax = ymax if ymax is not None else head_ymax

            print("简单字形指标 (通过 calcBounds 获取):")
            print("  xMin:", xmin, "  xMax:", xmax)
            print("  yMin:", ymin, "  yMax:", ymax)

            from fontTools.pens.recordingPen import RecordingPen
            pen = RecordingPen()
            charString.draw(pen)
            print("绘制指令:")
            import pprint
            pprint.pprint(pen.value)
        except Exception as e:
            print("处理 CFF 字形时出错：", e)
        print("CFF 字形对象详细信息:")
        try:
            import pprint
            pprint.pprint(dir(charString))
        except Exception as e:
            print("无法获取详细信息：", e)
    else:
        print("该字体既不含 glyf 表，也不含 CFF 表。")
    print("-----------------------------------")

def main():
    if len(sys.argv) < 2:
        print("使用方法: python fontexamine.py <font_path>")
        sys.exit(1)
    
    font_path = sys.argv[1]
    if not os.path.exists(font_path):
        print("字体文件不存在：", font_path)
        sys.exit(1)
    
    ext = os.path.splitext(font_path)[1].lower()
    # 如果是 TTC 或 OTC 集合，遍历集合中的所有字体
    if ext in [".ttc", ".otc"]:
        try:
            collection = TTCollection(font_path)
        except Exception as e:
            print("加载字体集合失败：", e)
            sys.exit(1)
        for index, font in enumerate(collection.fonts):
            print("========== 字体集合中字体索引 {} ==========".format(index))
            print_global_info(font)
            print_gidx_info(font, 10)
    else:
        try:
            font = TTFont(font_path)
        except Exception as e:
            print("加载字体失败：", e)
            sys.exit(1)
        print_global_info(font)
        print_gidx_info(font, 10)

if __name__ == "__main__":
    main()