default:
	mkdir -p ./build
	gcc desktop.c frun.c -lX11 -o ./build/frun

run:
	./build/frun
