
all:
	gcc AMF_client.c -o AMF -lpthread
	gcc UE_client.c -o UE -lpthread
	gcc gNB_sever.c -o gNB -lpthread
clean:
	rm -rf AMF
	rm -rf UE
	rm -rf gNB