# inject_sdkconfig_defaults.py
Import("env")
import os

defaults_file = os.path.join(env["PROJECT_DIR"], "sdkconfig.defaults")
if os.path.exists(defaults_file):
    env.Append(
        CPPDEFINES=["SDKCONFIG_DEFAULTS=\"" + defaults_file + "\""]
    )
    env.Append(
        CMAKE_DEFINES={"SDKCONFIG_DEFAULTS": defaults_file}
    )
else:
    print("sdkconfig.defaults not found")
