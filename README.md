# arduino-home-control-system
A control system that can be used to configure devices within a household and schedule them using a simple ui

On first boot the Serial monitor will print “ENHANCED:” followed by the features that have
been implemented. If the EEPROM contains a valid house structure then “VALID” will be
printed to the serial monitor on the next line.

Uncommenting line 699 will manually configure the house with the floors, rooms, etc.. that
are shown in the coursework spec.

If the house has already been set with floors then on the arduino LCD screen the text
“House/” will be printed, with the name of the first room on the next line.

If no floors have been added to the house then a warning message will be printed showing
that there are no floors in the house.
Using the UP, DOWN, LEFT and RIGHT buttons on the arduino you will be able to navigate

the menu:
 The UP button will take you to the previous menu option
 The DOWN button will take you to the next menu option
 The LEFT button will take you to the previous menu stage
 The RIGHT button will take you to the next menu stage
 If on the “Send values” menu option, the SELECT button will send the current
structure of the house to the serial monitor.

Sending a ‘Q’ to the serial monitor followed by a new line will display the structure of the
house to the serial monitor.

Sending an ‘M’ to the serial monitor followed by a new line will display “Data:” followed by
the current free RAM to the serial monitor.

Sending an ‘S’ to the serial monitor followed by a series of descriptors will activate the soft
menu and add the described floors, rooms, etc.. to the house (if they are valid). 
