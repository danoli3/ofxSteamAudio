#include "ofMain.h"
#include "ofAppNoWindow.h"
#include "ofApp.h"

int main() {
	ofAppNoWindow window;
	ofSetupOpenGL(&window, 64, 64, OF_WINDOW);
	return ofRunApp(new ofApp());
}
