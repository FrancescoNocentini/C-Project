PFC_Disconnect_Switch: transducers PFC1 PFC2 PFC3 WES generatoreFallimenti
	cc PFC_Disconnect_Switch.c -o PFC_Disconnect_Switch
transducers:
	cc transducers.c -o transducers
PFC1:
	cc PFC1.c -o PFC1 -lm
PFC2:
	cc PFC2.c -o PFC2 -lm
PFC3:
	cc PFC3.c -o PFC3 -lm
WES:
	cc WES.c -o WES
generatoreFallimenti:
	cc generatoreFallimenti.c -o generatoreFallimenti
clean:
	unlink socketPFC1
	unlink pipePFC2
	rm *.txt *.log
	rm -f "transducers"
	rm -f "PFC1"
	rm -f "PFC2"
	rm -f "PFC3"
	rm -f "WES"
	rm -f "generatoreFallimenti"
	rm -f "PFC_Disconnect_Switch"
