Import("env")

env.Replace(PROGNAME="%s" % env.GetProjectOption("firmware_name"))