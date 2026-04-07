# Скрипт объединения всех CSS/JS в index.html для ESP32 HTTPS
# Использование: python bundle_html.py

import re
import os

data_dir = "data"
html_file = os.path.join(data_dir, "index.html")
output_file = os.path.join(data_dir, "index_bundle.html")

# Читаем основной HTML
with open(html_file, 'r', encoding='utf-8') as f:
    html = f.read()

# Функция для вставки CSS
def inline_css(match):
    css_file = match.group(1)
    css_path = os.path.join(data_dir, css_file)
    if os.path.exists(css_path):
        with open(css_path, 'r', encoding='utf-8') as f:
            return f'<style>\n{f.read()}\n</style>'
    print(f"WARNING: CSS file not found: {css_file}")
    return match.group(0)

# Функция для вставки JS
def inline_js(match):
    js_file = match.group(1)
    js_path = os.path.join(data_dir, js_file)
    if os.path.exists(js_path):
        with open(js_path, 'r', encoding='utf-8') as f:
            return f'<script>\n{f.read()}\n</script>'
    print(f"WARNING: JS file not found: {js_file}")
    return match.group(0)

# Заменяем <link rel="stylesheet" href="*.css"> на <style>...</style>
html = re.sub(r'<link\s+rel="stylesheet"\s+href="([^"]+\.css)"[^>]*>', inline_css, html)

# Заменяем <script src="*.js"></script> на <script>...</script>
html = re.sub(r'<script\s+src="([^"]+\.js)"[^>]*>\s*</script>', inline_js, html)

# Записываем bundle
with open(output_file, 'w', encoding='utf-8') as f:
    f.write(html)

print(f"✅ Created: {output_file}")
print(f"📊 Original size: {os.path.getsize(html_file):,} bytes")
print(f"📊 Bundled size: {os.path.getsize(output_file):,} bytes")
print("\n📋 Теперь скопируйте index_bundle.html на SD карту как /www/index.html")
