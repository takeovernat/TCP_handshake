port=7777
serv:
	gcc -o serv server.c
	./serv $(port)
cli:
	gcc -o cli client.c
	./cli $(port)
clean: 
	rm cli
	rm serv
	rm server.log
	rm client.log

