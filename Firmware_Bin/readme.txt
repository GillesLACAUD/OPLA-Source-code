V14 (29.01.22)
- Route input Jack to Output Jack (only for ES8388 codec)
	- Very funny to play with you phone connect to the input jack
- Set the delay to go back to the master screen in a binary file /System/BackDelay.cfg
	- Just create the file and write one byte between 0 to 255 in second
	- If the file did not exist the delay is set to the default value 10s
- Add the flashdownload tool to the github
