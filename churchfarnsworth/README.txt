How to run application:

1) make sure the "churchfarnsworth" folder is in "orts/trunk/" directory.

2) Type "make MODE=opt churchfarnsworth". The application should build without incident.

3) In one terminal window, start a server using the script "tournament-2008/game4_orts" 
or the command "bin/orts -game4 -bp tournament-2008/game4.bp -x 64 -y 48 -fplat 0 -nfog -nplayers 2".

In a second terminal window, start a client with the command "bin/lab2.template -usegfx -p tournament-2008/ortsg -nfog -refresh 0 -nosound".
This will start an AI client with an associated ORTSG window.

In a third terminal window, start another client with the command "bin/lab2.template".
This will start an AI client without a graphical display.

4) Enjoy :)