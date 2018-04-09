# Derived from http://invent.module143.com/daskal_tutorial/raspberry-pi-3-wireless-pi-to-pi-python-communication-with-nrf24l01/
apt-get -y install unzip
apt-get -y install python3-dev
apt-get -y install vim
apt-get -y install python3-rpi.gpio
apt-get -y install python3-pip
apt-get -y install python3-alsaaudio
apt-get -y install python3-smbus
apt-get install libasound2-dev
wget https://github.com/Gadgetoid/py-spidev/archive/master.zip
unzip master.zip
rm master.zip
cd py-spidev-master
sudo python3 setup.py install
cd ..
git clone https://github.com/Blavery/lib_nrf24
cp lib_nrf24/lib_nrf24.py src/common/
