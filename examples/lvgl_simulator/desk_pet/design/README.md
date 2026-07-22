# Pro Desk Pet — Design Resources

## Prototypes

| File | Description | Resolution |
|------|-------------|------------|
| `hmi_prototype.html` | Smart Controller HMI — industrial dashboard | 480×320 |
| `desk_pet_prototype.html` | Desk Pet full UI — 5 screens with animations | 480×320 |

## How to use

Open any `.html` file directly in a browser. The prototypes use MDI icons via CDN — internet connection required for icon display.

## Design Tokens

All colors use CSS variables (defined in `:root`), directly mappable to LVGL `lv_color_hex()`:

| Variable | Hex | LVGL C | Usage |
|----------|-----|--------|-------|
| `--bg` | `#0b1120` | `lv_color_hex(0x0b1120)` | Page background |
| `--bar-bg` | `#111c30` | `lv_color_hex(0x111c30)` | Status bar |
| `--panel-bg` | `#172033` | `lv_color_hex(0x172033)` | Info panel |
| `--card-bg` | `#1c273b` | `lv_color_hex(0x1c273b)` | Card / item bg |
| `--accent` | `#22d3ee` | `lv_color_hex(0x22d3ee)` | Primary accent (cyan) |
| `--accent2` | `#fb923c` | `lv_color_hex(0xfb923c)` | Secondary accent (orange) |
| `--green` | `#22c55e` | `lv_color_hex(0x22c55e)` | WiFi / signal |
| `--red` | `#ef4444` | `lv_color_hex(0xef4444)` | Alarm / error |
| `--yellow` | `#facc15` | `lv_color_hex(0xfacc15)` | Weather / mood |
| `--purple` | `#a78bfa` | `lv_color_hex(0xa78bfa)` | Pressure / tertiary |
| `--label` | `#9ca3af` | `lv_color_hex(0x9ca3af)` | Secondary text |
| `--desc` | `#94a3b8` | `lv_color_hex(0x94a3b8)` | Description text |

## Icon Mapping

MDI web font (`mdi-*` classes in HTML) ↔ LVGL font glyphs (`mdi_icons_20.c`):

| HTML class | LVGL macro | Unicode |
|-----------|-----------|---------|
| `mdi-thermometer` | `MDI_THERMOMETER` | `U+F050F` |
| `mdi-water-percent` | `MDI_WATER_PERCENT` | `U+F058E` |
| `mdi-weather-sunny` | `MDI_WEATHER` | `U+F0595` |
| `mdi-signal` | `MDI_SIGNAL` | `U+F04A2` |
| `mdi-wifi` | LVGL built-in | `LV_SYMBOL_WIFI` |
| `mdi-brightness-7` | `MDI_BRIGHTNESS_7` | `U+F00E0` |
| `mdi-lightbulb` | `MDI_LIGHTBULB` | `U+F0335` |
| `mdi-alert-outline` | `MDI_ALERT_OUTLINE` | `U+F002A` |
| `mdi-chip` | `MDI_CHIP` | `U+F061A` |

## Fonts

| Font | File | Usage |
|------|------|-------|
| MDI Icons 20px | `res/fonts/mdi_icons_20.c` | All icon glyphs |
| Montserrat 12-48 | LVGL built-in | Latin text / numbers |
| Source Han Sans SC | LVGL built-in (future) | Chinese text |

## Adding new MDI icons

1. Find the icon at https://pictogrammers.com/library/mdi/
2. Note the Unicode code point
3. Regenerate `mdi_icons_20.c` with the new code point added to the `-r` flag
4. Add a `#define MDI_*` macro in `res/fonts/mdi_icons.h`
