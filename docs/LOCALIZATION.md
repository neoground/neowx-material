# Localization & Language Guide

NeoWX Material ships with complete, professionally maintained translations for **12 languages**, localized date/time support, and customizable chart/card date formats.

---

## 1. Supported Languages

All translation dictionaries live in `skins/neowx-material/lang/`:

| Code | Language | File |
| :--- | :--- | :--- |
| `ca` | Catalan | `lang/ca.conf` |
| `da` | 🇩🇰 Danish | `lang/da.conf` |
| `de` | 🇩🇪 German | `lang/de.conf` |
| `en` | 🇬🇧 English | `lang/en.conf` |
| `es` | 🇪🇸 Spanish | `lang/es.conf` |
| `fi` | 🇫🇮 Finnish | `lang/fi.conf` |
| `fr` | 🇫🇷 French | `lang/fr.conf` |
| `it` | 🇮🇹 Italian | `lang/it.conf` |
| `nl` | 🇳🇱 Dutch | `lang/nl.conf` |
| `pl` | 🇵🇱 Polish | `lang/pl.conf` |
| `se` | 🇸🇪 Swedish | `lang/se.conf` |
| `sk` | 🇸🇰 Slovak | `lang/sk.conf` |

---

## 2. Setting Your Language in `weewx.conf`

Set the `lang` option under `[StdReport]` for your NeoWX Material report in `weewx.conf`:

```ini
[StdReport]
    [[neowx-material]]
        skin = neowx-material
        enable = true
        lang = de  # Options: ca, da, de, en, es, fi, fr, it, nl, pl, se, sk
```

Also set `language` under `[Extras]` in `skin.conf` (or via your `skin.conf.patch` file) so client-side scripts (Moment.js, ApexCharts, and the interactive date picker) load the matching locale:

```ini
[Extras]
    language = de
```

Reload or restart WeeWX after changing `weewx.conf`:

```bash
sudo systemctl reload weewx
```

---

## 3. System Locale Setup (Date & Weekday Names)

WeeWX uses your operating system's Python locale for server-rendered month and weekday names (`strftime`). Make sure the corresponding UTF-8 locale is installed on your host or container:

1. **Check installed locales:**
   ```bash
   locale -a
   ```

2. **Install your locale if missing:**
   ```bash
   # Debian / Ubuntu
   sudo dpkg-reconfigure locales

   # RHEL / CentOS / Fedora
   sudo locale-gen de_DE.UTF-8
   ```

3. **Configure the WeeWX systemd service (if needed):**
   Edit `/etc/systemd/system/weewx.service` (or use `sudo systemctl edit weewx`) and set `LANG` under `[Service]`:
   ```ini
   [Service]
   Environment="LANG=de_DE.UTF-8"
   ```
   Then reload systemd and restart WeeWX:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl restart weewx
   ```

---

## 4. Customizing Date & Time Formats

Beyond selecting a language, you can customize how dates and times are formatted on the header, cards (`strftime`), and interactive charts (`moment.js`) in `skin.conf` under `[Extras] → [[Formatting]]`.

👉 See the **[Date & Time Formatting Guide](DATE-FORMATTING.md)** for a complete token cheat-sheet and practical examples.

---

## 5. Adding or Updating a Language

Contributions for new languages or translation improvements are very welcome!

1. Copy `skins/neowx-material/lang/en.conf` to `skins/neowx-material/lang/<your_code>.conf`.
2. Translate the strings under `[Units]`, `[Labels]`, `[Almanac]`, and `[Texts]`.
3. Register the new file in `install.py` under `("skins/neowx-material/lang", [...])`.
4. Submit a pull request on [GitHub](https://github.com/seehase/neowx-material/pulls).

For more background on how WeeWX handles localization files, see the [WeeWX Localization Documentation](https://weewx.com/docs/5.1/custom/localization/).

