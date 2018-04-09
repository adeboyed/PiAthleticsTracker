# Derived from http://invent.module143.com/daskal_tutorial/raspberry-pi-3-wireless-pi-to-pi-python-communication-with-nrf24l01/
sudo apt-get -y install unzip
sudo apt-get -y install python3-dev
sudo apt-get -y install vim
sudo apt-get -y install python3-rpi.gpio
sudo apt-get -y install python-alsaaudio
sudo apt-get -y install python3-smbus
wget https://github.com/Gadgetoid/py-spidev/archive/master.zip
unzip master.zip
rm master.zip
cd py-spidev-master
sudo python3 setup.py install
cd ..
git clone https://github.com/Blavery/lib_nrf24
cp lib_nrf24/lib_nrf24.py src/common/
