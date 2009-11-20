////////////////////////////////////////////////////////////////////////////////////////////////////////
Team Church-Farnsworth Game 4 Skirmish for CMPUT 396 ORTS Project at the University of Alberta Fall 2009

****Team members****:
Matthew Church
Evan Farnsworth
********************

Check out the Team Wiki at http://code.google.com/p/orts3cf/w/list
////////////////////////////////////////////////////////////////////////////////////////////////////////


Hey Evan,
This is all based off the tutorial in CMPUT396 Newsgroup Lab 2.
Right now, it is a direct copy of labs2.template/template_main.C (our churchfarnsworth_main.C)
This is where we will be editing the AI, most of the other classes are just helper classes needed to run the program


How to run application:

1) make sure the "churchfarnsworth" folder is in "orts/trunk/apps" directory.

2) From the "orts/trunk/" directory, type "make MODE=opt churchfarnsworth". The application should build without incident.

3) 
a) In one terminal window, start a server using the script "tournament-2008/game4_orts" 
or the command "bin/orts -game4 -bp tournament-2008/game4.bp -x 64 -y 48 -fplat 0 -nfog -nplayers 2".

b) In a second terminal window, start a client with the command
"bin/churchfarnsworth -disp -p tournament-2008/ortsg -nfog -refresh 0 -nosound".
This will start an AI client with an associated ORTSG window.

c) In a third terminal window, start another client with the command "bin/churchfarnsworth".
This will start an AI client without a graphical display.

In summary, you can launch the client in three different ways:

No display: bin/churchfarnsworth
2D display: bin/churchfarnsworth -disp
3D display: bin/churchfarnsworth -usegfx -p tournament-2008/ortsg -nfog -refresh 0 -nosound

4) Enjoy :)