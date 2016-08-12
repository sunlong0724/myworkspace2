#include "DeckLinkIODevice.h"

int main(int argc, char** argv) {

	CDeckLinkInputDevice di;
	di.CreateObjects(0);
	di.start_capture();
	di.stop_capture();
	di.DestroyObjects();

	return 0;
}