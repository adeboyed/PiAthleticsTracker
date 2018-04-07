# PiAthleticsTracker


## Important Information 


## Steps 
1. Establish communication between the Raspberry Pis
2. Setup light gate at the finish line
3. Sync up time 
4. Waits for start

START RACE
1. Click start on the webpage
2. Starts a countdown thing including "Ready, Set, Go" out of the audio of the client
3. Person races
4. Passes light gate
5. Client asks for time from server

## Tech Used
- 2x Raspberry Pis (1 v3, 1 v2)
- 2x NRF24L01+ for communication between the Pis
- 1x TCS34725 RGB Sensor for Light Gate
- 2x Camera Tripod for holding the parts of the light gate
- 1x Laser Pointer

## Raspberry Pi Roles
### Base Pi
- Raspberry Pi 3
- Located at the start of the race

### Client Pi
- Raspberry Pi 2
- Located at the end of the race
