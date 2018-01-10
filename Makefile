default:
	mkdir -p ./build
	gcc desktop.c frun.c -g -lX11 -o ./build/frun

run:
	./build/frun
