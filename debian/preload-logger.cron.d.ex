#
# Regular cron jobs for the preload-logger package
#
0 4	* * *	root	[ -x /usr/bin/preload-logger_maintenance ] && /usr/bin/preload-logger_maintenance
