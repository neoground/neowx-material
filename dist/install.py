# installer for the neowx-material skin

from setup import ExtensionInstaller

def loader():
    return BasicInstaller()

class BasicInstaller(ExtensionInstaller):
    def __init__(self):
        super(BasicInstaller, self).__init__(
            version="1.0",
            name='neowx-material',
            description='The most versatile and modern weewx skin',
            author="Neoground GmbH",
            author_email="weather@neoground.com",
            config={
                'StdReport': {
                    'StandardReport': {
                        'skin':'neowx-material'
                    }
                }
            },
            files=[('skins/neowx-material',
                    ['skins/neowx-material/archive.html.tmpl',
                    # TODO
                    'skins/neowx-material/archive/NOAA-%Y.txt.tmpl',
                    'skins/neowx-material/archive/NOAA-%Y-%m.txt.tmpl',
                    'skins/neowx-material/weather-icons/css/weather-icons.css',
                    'skins/neowx-material/weather-icons/css/weather-icons.min.css',
                    'skins/neowx-material/weather-icons/css/weather-icons-wind.css',
                    'skins/neowx-material/weather-icons/css/weather-icons-wind.min.css',
                    'skins/neowx-material/weather-icons/font/weathericons-regular-webfont.eot',
                    'skins/neowx-material/weather-icons/font/weathericons-regular-webfont.svg',
                    'skins/neowx-material/weather-icons/font/weathericons-regular-webfont.ttf',
                    'skins/neowx-material/weather-icons/font/weathericons-regular-webfont.woff',
                    'skins/neowx-material/weather-icons/font/weathericons-regular-webfont.woff2']),
            ]
            )