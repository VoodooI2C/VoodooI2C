#<cldoc:Supported Gestures>

&#8291;

VoodooI2C currently uses CSGesture (developed by [@coolstar](https://github.com/coolstar)) to implement multitouch. Many of the gestures supported by CSGesture are configurable via the System Preferences Trackpad pane. However some of them are hardcoded and will not respect the values set in the preference pane. Here is a full list of the supported gestures (and when relevant, the keystroke they send to OS X):

	1. Tap to click
	2. Two finger scrolling
	3. Two finger movement with thumb rejection
	3. Three finger swipe in up, down, left right directions: Alt + Up/Left/Right/Down
	4. Four finger swipe in up, down, left right directions
		- Down - Command + W
		- Left - Command + Q
		- Right - Show Desktop
		- Up - Command + F11

All three and four finger gestures (except the CMD+W/Q) ones can be configured in the Keyboard Shortcuts PrefPane and the Mission Control PrefPane.