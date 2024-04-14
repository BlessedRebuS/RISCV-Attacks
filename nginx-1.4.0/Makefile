
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/home/pdifazio/nginx/nginx-1.4.0/install -t

	kill -USR2 `cat /home/pdifazio/nginx/nginx-1.4.0/install/nginx.pid`
	sleep 1
	test -f /home/pdifazio/nginx/nginx-1.4.0/install/nginx.pid.oldbin

	kill -QUIT `cat /home/pdifazio/nginx/nginx-1.4.0/install/nginx.pid.oldbin`
