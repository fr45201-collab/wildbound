from PIL import Image

sheet = Image.open("assets/external/kenney/roguelike_sheet_raw.png").convert("RGBA")

# Helper to crop tile (col, row)
def get_tile(col, row):
    x = col * 17
    y = row * 17
    return sheet.crop((x, y, x + 16, y + 16))

# Let's inspect some candidate coordinates:
# Top rows often have terrain:
print("Sheet size:", sheet.size)
# Sample a montage of cols 0..9 and rows 0..9
montage = Image.new("RGBA", (10 * 18, 10 * 18), (30, 30, 30, 255))
for r in range(10):
    for c in range(10):
        t = get_tile(c, r)
        montage.paste(t, (c * 18, r * 18))

montage.save("tools/asset_pipeline/kenney_sample_0_9.png")
print("Saved sample 0..9")

