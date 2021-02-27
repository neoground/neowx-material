NeoWX Material
The most versatile and modern weewx skin

---

Installation instructions

1) install the extension

wee_extension --install=/absolute/path/to/current/directory

2) restart weeWX

sudo service weewx restart

---

Manual installation instructions

1) copy files to the weeWX skins directory

cp -rp skins/neowx-material /home/weewx/skins

2) in the weeWX configuration file, add / change a report specifing this skin

[StdReport]
    ...
    [[StandardReport]]
        skin = neowx-material

3) restart weeWX

sudo service weewx restart

---

More information: https://projects.neoground.com/neowx-material

Github: https://github.com/neoground/neowx-material
