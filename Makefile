default:
	mkdir -p ./build
	gcc frun.c -lX11 -o ./build/frun

run:
	./build/frun
