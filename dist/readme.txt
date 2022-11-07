+-------------------------------------------------------------------------+
|                                                                         |
|       ()    ()   _   _                                            _     |
|                 | \ | | ___  ___   __ _ _ __ ___  _   _ _ __   __| |    |
|    ()    ()     |  \| |/ _ \/ _ \ / _` | '__/ _ \| | | | '_ \ / _` |    |
|                 | |\  |  __/ (_) | (_| | | | (_) | |_| | | | | (_| |    |
|       ()    ()  |_| \_|\___|\___/ \__, |_|  \___/ \__,_|_| |_|\__,_|    |
|                                   |___/                                 |
|    ()    ()                                                  G m b H    |
|                                                                         |
|              weather (at) neoground.com        https://neoground.com    |
|                                                                         |
+-------------------------------------------------------------------------+

+-------------------------------------------------------------------------+
|                                                                         |
|                N E O W X    M A T E R I A L    S K I N                  |
|                                                                         |
|                              R E A D   M E                              |
|                                                                         |
+-------------------------------------------------------------------------+

+-------------------------------------------------------------------------+
|                                                                         |
| Project information:    https://neoground.com/projects/neowx-material   |
|                                                                         |
|       Documentation:    https://neoground.com/docs/neowx-material       |
|                                                                         |
|              Github:    https://github.com/neoground/neowx-material     |
|                                                                         |
+-------------------------------------------------------------------------+

+-------------------------------------------------------------------------+
|                                                                         |
| Installation instructions                                               |
|                                                                         |
| 1) install the extension                                                |
|    > wee_extension --install=/absolute/path/to/current/directory        |
|                                                                         |
| 2) restart WeeWX                                                        |
|    > sudo service weewx restart                                         |
|                                                                         |
+-------------------------------------------------------------------------+

+-------------------------------------------------------------------------+
|                                                                         |
| Manual installation instructions                                        |
|                                                                         |
| 1) copy files to the weeWX skins directory                              |
|    > cp -rp skins/neowx-material /home/weewx/skins                      |
|                                                                         |
| 3) copy /bin/user/historygenerator.py to                                |
|              $WEEWX_ROOT/user. default: /usr/share/weewx/user           |
|                                                                         |
| 3) in the weeWX configuration file, add / change a report and set       |
|    neowx-material as its skin                                           |
|    [StdReport]                                                          |
|        ...                                                              |
|            [[StandardReport]]                                           |
|                skin = neowx-material                                    |
|                                                                         |
| 3) restart WeeWX                                                        |
|    > sudo service weewx restart                                         |
|                                                                         |
+-------------------------------------------------------------------------+
