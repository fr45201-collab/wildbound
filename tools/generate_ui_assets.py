import os
import json
import math
from PIL import Image, ImageDraw, ImageFilter

CANVAS_W = 640
CANVAS_H = 360

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

# =============================================================================
# 1. BACKGROUNDS (10 Designs @ 640x360)
# =============================================================================

def generate_backgrounds():
    bg_dir = "assets/ui/backgrounds"
    ensure_dir(bg_dir)

    # Helper to create gradient
    def vertical_gradient(draw, top_color, bottom_color, w=CANVAS_W, h=CANVAS_H):
        for y in range(h):
            factor = y / float(h - 1)
            r = int(top_color[0] + factor * (bottom_color[0] - top_color[0]))
            g = int(top_color[1] + factor * (bottom_color[1] - top_color[1]))
            b = int(top_color[2] + factor * (bottom_color[2] - top_color[2]))
            draw.line([(0, y), (w, y)], fill=(r, g, b, 255))

    # BG-001: Main Menu
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (18, 22, 38), (8, 10, 16))
    # Moonlit clouds and mountains
    draw.ellipse((480, 40, 560, 120), fill=(240, 245, 255, 40))
    draw.ellipse((485, 45, 555, 115), fill=(250, 252, 255, 220)) # Moon
    # Mountain silhouettes
    draw.polygon([(0, 280), (120, 180), (260, 260), (420, 150), (560, 240), (640, 190), (640, 360), (0, 360)], fill=(22, 28, 45, 255))
    draw.polygon([(0, 310), (180, 240), (320, 290), (480, 230), (640, 280), (640, 360), (0, 360)], fill=(12, 16, 26, 255))
    # Pine forest treeline in foreground
    for x in range(0, CANVAS_W, 14):
        th = 40 + (x % 5) * 8
        draw.polygon([(x, 360), (x + 7, 360 - th), (x + 14, 360)], fill=(7, 10, 18, 255))
    im.save(os.path.join(bg_dir, "bg_main_menu.png"))

    # BG-002: Character Selection
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (24, 28, 42), (14, 16, 24))
    # Stone arches and central illuminated pedestal
    draw.rectangle((100, 40, 130, 360), fill=(35, 40, 58, 255))
    draw.rectangle((510, 40, 540, 360), fill=(35, 40, 58, 255))
    draw.polygon([(100, 40), (130, 40), (320, 15), (510, 40), (540, 40), (320, 0)], fill=(45, 50, 72, 255))
    # Pedestal light shaft
    draw.polygon([(260, 0), (380, 0), (440, 320), (200, 320)], fill=(180, 210, 255, 30))
    # Stone floor slabs
    for y in range(290, 360, 14):
        draw.line([(0, y), (640, y)], fill=(40, 46, 68, 255))
    im.save(os.path.join(bg_dir, "bg_character_select.png"))

    # BG-003: Wardrobe
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (35, 28, 24), (18, 14, 12))
    # Wood paneling and banner tapestries
    for x in range(0, CANVAS_W, 32):
        draw.line([(x, 0), (x, 280)], fill=(28, 22, 18, 255))
    # Hanging tapestries
    draw.rectangle((60, 20, 120, 220), fill=(110, 30, 40, 220), outline=(190, 150, 60, 255))
    draw.rectangle((520, 20, 580, 220), fill=(30, 75, 110, 220), outline=(190, 150, 60, 255))
    # Parquet wood floor
    for y in range(280, 360, 12):
        draw.line([(0, y), (640, y)], fill=(60, 40, 25, 255))
    im.save(os.path.join(bg_dir, "bg_wardrobe.png"))

    # BG-004: Inventory
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (20, 22, 28), (10, 12, 16))
    # Subtle parchment grid texture
    for x in range(40, CANVAS_W - 40, 40):
        draw.line([(x, 20), (x, 340)], fill=(28, 32, 42, 120))
    for y in range(30, CANVAS_H - 30, 35):
        draw.line([(40, y), (600, y)], fill=(28, 32, 42, 120))
    im.save(os.path.join(bg_dir, "bg_inventory.png"))

    # BG-005: Settings
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (16, 20, 30), (8, 10, 15))
    # Geometric blueprint lines
    draw.rectangle((30, 20, 610, 340), outline=(32, 45, 68, 160), width=1)
    draw.line([(30, 50), (610, 50)], fill=(32, 45, 68, 160))
    draw.line([(180, 50), (180, 340)], fill=(32, 45, 68, 160))
    im.save(os.path.join(bg_dir, "bg_settings.png"))

    # BG-006: Loading Screen
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (22, 18, 14), (12, 10, 8))
    # Weathered map continents and compass rose
    draw.ellipse((270, 130, 370, 230), outline=(130, 100, 60, 100), width=2)
    draw.line([(320, 100), (320, 260)], fill=(180, 140, 80, 150), width=2)
    draw.line([(240, 180), (400, 180)], fill=(180, 140, 80, 150), width=2)
    # Vignette border
    draw.rectangle((0, 0, 639, 359), outline=(5, 5, 5, 200), width=8)
    im.save(os.path.join(bg_dir, "bg_loading.png"))

    # BG-007: Lobby
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (14, 18, 32), (8, 12, 20))
    # Starry night + camp campfire glow
    draw.ellipse((270, 260, 370, 360), fill=(255, 120, 30, 60))
    draw.polygon([(300, 340), (320, 290), (340, 340)], fill=(255, 160, 40, 220))
    # Tents
    draw.polygon([(80, 340), (150, 240), (220, 340)], fill=(25, 32, 48, 255))
    draw.polygon([(420, 340), (490, 240), (560, 340)], fill=(25, 32, 48, 255))
    im.save(os.path.join(bg_dir, "bg_lobby.png"))

    # BG-008: Match Results
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (28, 24, 38), (14, 12, 18))
    # Victorious heraldic banners and golden ray bursts
    for a in range(-60, 70, 15):
        rad = math.radians(a + 90)
        draw.line([(320, 0), (320 + int(math.cos(rad) * 450), int(math.sin(rad) * 450))], fill=(255, 215, 80, 20), width=4)
    im.save(os.path.join(bg_dir, "bg_match_results.png"))

    # BG-009: Pause Menu
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H), (10, 12, 18, 225))
    draw = ImageDraw.Draw(im)
    # Subtle ornate gold corner accents
    draw.line([(20, 20), (80, 20)], fill=(210, 175, 75, 200), width=2)
    draw.line([(20, 20), (20, 80)], fill=(210, 175, 75, 200), width=2)
    draw.line([(620, 20), (560, 20)], fill=(210, 175, 75, 200), width=2)
    draw.line([(620, 20), (620, 80)], fill=(210, 175, 75, 200), width=2)
    draw.line([(20, 340), (80, 340)], fill=(210, 175, 75, 200), width=2)
    draw.line([(20, 340), (20, 280)], fill=(210, 175, 75, 200), width=2)
    draw.line([(620, 340), (560, 340)], fill=(210, 175, 75, 200), width=2)
    draw.line([(620, 340), (620, 280)], fill=(210, 175, 75, 200), width=2)
    im.save(os.path.join(bg_dir, "bg_pause_menu.png"))

    # BG-010: Game Over
    im = Image.new("RGBA", (CANVAS_W, CANVAS_H))
    draw = ImageDraw.Draw(im)
    vertical_gradient(draw, (35, 12, 16), (15, 6, 8))
    # Falling embers
    for i in range(40):
        ex = (i * 17) % CANVAS_W
        ey = (i * 29) % CANVAS_H
        draw.rectangle((ex, ey, ex + 2, ey + 2), fill=(255, 90, 40, 180))
    im.save(os.path.join(bg_dir, "bg_game_over.png"))

    print(f"Generated 10 background assets in {bg_dir}")

# =============================================================================
# 2. PANELS (Nine-Slice Scalable Frames)
# =============================================================================

def generate_panels():
    panel_dir = "assets/ui/panels"
    ensure_dir(panel_dir)

    # 1. panel_main_64.png (64x64): Dark slate fill with 4px golden bevel border and ornate corners
    im = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    # Background fill
    draw.rectangle((4, 4, 59, 59), fill=(18, 22, 34, 240))
    # Outer dark edge
    draw.rectangle((0, 0, 63, 63), outline=(10, 12, 18, 255), width=1)
    # Inner gold border
    draw.rectangle((2, 2, 61, 61), outline=(180, 145, 65, 255), width=2)
    # Highlight rim
    draw.line([(3, 3), (60, 3)], fill=(240, 215, 120, 255))
    draw.line([(3, 3), (3, 60)], fill=(240, 215, 120, 255))
    # Corners
    for cx, cy in [(2, 2), (58, 2), (2, 58), (58, 58)]:
        draw.rectangle((cx, cy, cx + 3, cy + 3), fill=(255, 230, 140, 255))
    im.save(os.path.join(panel_dir, "panel_main_64.png"))

    # 2. panel_dark_64.png (64x64): Inset darker container
    im = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    draw.rectangle((2, 2, 61, 61), fill=(10, 12, 18, 230), outline=(40, 48, 65, 255), width=2)
    draw.rectangle((4, 4, 59, 59), outline=(15, 18, 26, 255), width=1)
    im.save(os.path.join(panel_dir, "panel_dark_64.png"))

    # 3. panel_tooltip_32.png (32x32): Lightweight tooltip
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    draw.rectangle((1, 1, 30, 30), fill=(12, 14, 20, 245), outline=(140, 120, 80, 255), width=1)
    im.save(os.path.join(panel_dir, "panel_tooltip_32.png"))

    # 4. panel_card_48.png (48x48): Item/Cosmetic Card slot
    im = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    draw.rectangle((2, 2, 45, 45), fill=(22, 26, 38, 255), outline=(60, 70, 95, 255), width=2)
    draw.rectangle((4, 4, 43, 43), outline=(12, 14, 22, 255), width=1)
    im.save(os.path.join(panel_dir, "panel_card_48.png"))

    # 5. panel_hud_bar.png (128x16): Health/Stamina track & fill
    im = Image.new("RGBA", (128, 16), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)
    draw.rectangle((0, 0, 127, 15), fill=(15, 18, 25, 255), outline=(120, 100, 60, 255), width=1)
    # Red health gradient bar on top half
    draw.rectangle((2, 2, 60, 13), fill=(210, 40, 45, 255))
    draw.line([(2, 2), (60, 2)], fill=(255, 120, 120, 255))
    im.save(os.path.join(panel_dir, "panel_hud_bar.png"))

    print(f"Generated 5 reusable panel assets in {panel_dir}")

# =============================================================================
# 3. BUTTONS (Frames for 6 States: Normal, Hover, Pressed, Disabled, Selected, Focused)
# =============================================================================

def generate_buttons():
    btn_dir = "assets/ui/buttons"
    ensure_dir(btn_dir)

    # Each button frame is 96x32 with distinct styling
    styles = {
        "primary": {
            "normal": ((32, 40, 60, 255), (190, 155, 75, 255), (240, 215, 125, 255)),
            "hover": ((45, 58, 88, 255), (230, 195, 95, 255), (255, 240, 160, 255)),
            "pressed": ((22, 28, 42, 255), (140, 110, 50, 255), (100, 80, 35, 255)),
            "disabled": ((25, 28, 35, 200), (60, 65, 75, 200), (75, 80, 90, 200)),
            "selected": ((55, 70, 105, 255), (255, 225, 110, 255), (255, 250, 190, 255)),
            "focused": ((38, 48, 72, 255), (80, 180, 245, 255), (140, 220, 255, 255))
        },
        "destructive": {
            "normal": ((55, 25, 30, 255), (180, 60, 70, 255), (230, 100, 110, 255)),
            "hover": ((75, 35, 42, 255), (220, 80, 90, 255), (255, 130, 140, 255)),
            "pressed": ((38, 16, 20, 255), (130, 40, 50, 255), (90, 25, 32, 255)),
            "disabled": ((30, 24, 26, 200), (65, 50, 55, 200), (80, 65, 70, 200)),
            "selected": ((85, 40, 48, 255), (240, 100, 110, 255), (255, 150, 160, 255)),
            "focused": ((65, 30, 36, 255), (255, 120, 130, 255), (255, 180, 190, 255))
        },
        "confirm": {
            "normal": ((25, 55, 35, 255), (60, 175, 95, 255), (100, 225, 140, 255)),
            "hover": ((35, 75, 48, 255), (80, 215, 120, 255), (130, 255, 170, 255)),
            "pressed": ((18, 38, 24, 255), (40, 125, 68, 255), (30, 90, 50, 255)),
            "disabled": ((24, 30, 26, 200), (50, 65, 55, 200), (65, 80, 70, 200)),
            "selected": ((40, 85, 55, 255), (100, 235, 135, 255), (150, 255, 185, 255)),
            "focused": ((30, 65, 42, 255), (120, 245, 155, 255), (170, 255, 200, 255))
        }
    }

    for style_name, states in styles.items():
        for state_name, (fill_col, border_col, hi_col) in states.items():
            im = Image.new("RGBA", (96, 32), (0, 0, 0, 0))
            draw = ImageDraw.Draw(im)

            # Outer border
            draw.rectangle((0, 0, 95, 31), outline=(8, 10, 15, 255), width=1)
            # Body fill
            draw.rectangle((1, 1, 94, 30), fill=fill_col, outline=border_col, width=2)
            # Top highlight bevel
            if state_name != "pressed":
                draw.line([(2, 2), (93, 2)], fill=hi_col)
                draw.line([(2, 2), (2, 29)], fill=hi_col)
            else:
                # Inset shadow
                draw.line([(2, 2), (93, 2)], fill=(10, 12, 16, 200))
                draw.line([(2, 2), (2, 29)], fill=(10, 12, 16, 200))

            im.save(os.path.join(btn_dir, f"btn_{style_name}_{state_name}.png"))

    # Tab button (72x24)
    for state in ["normal", "hover", "selected"]:
        im = Image.new("RGBA", (72, 24), (0, 0, 0, 0))
        draw = ImageDraw.Draw(im)
        fill = (22, 28, 42, 255) if state == "normal" else ((38, 48, 72, 255) if state == "hover" else (50, 64, 96, 255))
        border = (120, 100, 60, 255) if state != "selected" else (240, 210, 110, 255)
        draw.rectangle((0, 0, 71, 23), fill=fill, outline=border, width=1)
        if state == "selected":
            draw.line([(1, 1), (70, 1)], fill=(255, 235, 140, 255), width=2)
        im.save(os.path.join(btn_dir, f"btn_tab_{state}.png"))

    print(f"Generated button state assets in {btn_dir}")

# =============================================================================
# 4. ICONS (25 UI Icons @ 24x24 px)
# =============================================================================

def generate_icons():
    icon_dir = "assets/ui/icons"
    ensure_dir(icon_dir)

    icons = [
        "icon_character", "icon_inventory", "icon_weapons", "icon_armor", "icon_clothing",
        "icon_cosmetics", "icon_backpacks", "icon_map", "icon_settings", "icon_audio",
        "icon_graphics", "icon_controls", "icon_health", "icon_stamina", "icon_items",
        "icon_consumables", "icon_collectibles", "icon_currency", "icon_notifications",
        "icon_confirm", "icon_error", "icon_pause", "icon_exit", "icon_search", "icon_nav"
    ]

    for name in icons:
        im = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
        draw = ImageDraw.Draw(im)
        C_GOLD = (235, 200, 100, 255)
        C_WHITE = (245, 245, 255, 255)
        C_RED = (235, 65, 75, 255)
        C_GREEN = (65, 215, 105, 255)
        C_BLUE = (80, 180, 245, 255)

        if "character" in name:
            draw.ellipse((8, 3, 15, 10), fill=C_GOLD) # Head
            draw.polygon([(6, 21), (12, 12), (17, 21)], fill=C_GOLD) # Body
        elif "inventory" in name:
            draw.rectangle((4, 6, 19, 19), fill=C_GOLD, outline=(140, 110, 50, 255))
            draw.line([(4, 11), (19, 11)], fill=(140, 110, 50, 255))
        elif "weapons" in name:
            draw.line([(4, 19), (19, 4)], fill=C_WHITE, width=2)
            draw.line([(6, 15), (9, 18)], fill=C_GOLD, width=2) # Guard
        elif "armor" in name:
            draw.polygon([(6, 5), (17, 5), (19, 12), (12, 21), (4, 12)], fill=C_BLUE)
        elif "clothing" in name:
            draw.polygon([(8, 4), (15, 4), (20, 9), (17, 12), (15, 10), (15, 20), (8, 20), (8, 10), (6, 12), (3, 9)], fill=C_GREEN)
        elif "cosmetics" in name:
            draw.ellipse((5, 5, 18, 18), outline=C_GOLD, width=2)
            draw.point((11, 11), fill=C_WHITE)
        elif "backpacks" in name:
            draw.rectangle((5, 6, 18, 19), fill=C_GOLD, outline=(120, 90, 40, 255))
            draw.rectangle((7, 8, 16, 12), fill=(160, 125, 60, 255))
        elif "map" in name:
            draw.polygon([(4, 6), (9, 4), (14, 6), (19, 4), (19, 19), (14, 17), (9, 19), (4, 17)], fill=C_GOLD)
        elif "settings" in name or "controls" in name:
            draw.ellipse((6, 6, 17, 17), fill=C_WHITE)
            draw.ellipse((9, 9, 14, 14), fill=(20, 24, 35, 255)) # Gear hole
        elif "audio" in name:
            draw.rectangle((5, 8, 9, 15), fill=C_WHITE)
            draw.polygon([(9, 8), (15, 4), (15, 19), (9, 15)], fill=C_WHITE)
            draw.arc((14, 6, 19, 17), -60, 60, fill=C_GOLD, width=2)
        elif "graphics" in name:
            draw.rectangle((4, 5, 19, 16), outline=C_WHITE, width=2)
            draw.line([(10, 16), (13, 16)], fill=C_GOLD, width=2)
            draw.line([(8, 19), (15, 19)], fill=C_GOLD, width=2)
        elif "health" in name:
            # Heart
            draw.polygon([(11, 19), (4, 12), (4, 7), (8, 4), (11, 7), (14, 4), (18, 7), (18, 12)], fill=C_RED)
        elif "stamina" in name:
            # Lightning bolt
            draw.polygon([(13, 3), (6, 12), (11, 12), (9, 21), (17, 10), (12, 10)], fill=C_GOLD)
        elif "items" in name or "consumables" in name:
            # Potion flask
            draw.rectangle((10, 4, 13, 7), fill=C_WHITE)
            draw.ellipse((6, 8, 17, 19), fill=C_RED)
        elif "collectibles" in name or "currency" in name:
            # Coin
            draw.ellipse((5, 5, 18, 18), fill=C_GOLD, outline=(180, 140, 50, 255))
            draw.line([(11, 8), (11, 15)], fill=(180, 140, 50, 255), width=2)
        elif "notifications" in name:
            # Bell
            draw.ellipse((8, 6, 15, 15), fill=C_GOLD)
            draw.rectangle((6, 12, 17, 16), fill=C_GOLD)
            draw.point((11, 18), fill=C_GOLD)
        elif "confirm" in name:
            # Checkmark
            draw.line([(4, 12), (9, 17), (19, 6)], fill=C_GREEN, width=3)
        elif "error" in name or "exit" in name:
            # Cross
            draw.line([(5, 5), (18, 18)], fill=C_RED, width=3)
            draw.line([(18, 5), (5, 18)], fill=C_RED, width=3)
        elif "pause" in name:
            # Two vertical bars
            draw.rectangle((6, 5, 9, 18), fill=C_WHITE)
            draw.rectangle((14, 5, 17, 18), fill=C_WHITE)
        elif "search" in name:
            # Magnifying glass
            draw.ellipse((5, 5, 14, 14), outline=C_WHITE, width=2)
            draw.line([(13, 13), (19, 19)], fill=C_WHITE, width=3)
        else:
            # Default diamond / compass
            draw.polygon([(11, 3), (19, 11), (11, 19), (3, 11)], fill=C_GOLD)

        im.save(os.path.join(icon_dir, f"{name}.png"))

    print(f"Generated 25 UI icon assets in {icon_dir}")

if __name__ == "__main__":
    generate_backgrounds()
    generate_panels()
    generate_buttons()
    generate_icons()

