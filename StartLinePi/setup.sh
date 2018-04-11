INSTALL_PATH="."
INSTALL_DIR="/rf24libs"

ROOT_PATH=${INSTALL_PATH}
ROOT_PATH+=${INSTALL_DIR}


apt-get -y install unzip
apt-get -y install vim
apt-get -y install libasound2-dev
apt-get -y install libncurses5-dev
apt-get -y install dnsmasq
git clone https://github.com/tmrh20/RF24.git ${ROOT_PATH}/RF24
sudo make install -B -C ${ROOT_PATH}/RF24
git clone https://github.com/tmrh20/RF24Network.git ${ROOT_PATH}/RF24Network
sudo make install -B -C ${ROOT_PATH}/RF24Network
git clone https://github.com/tmrh20/RF24Mesh.git ${ROOT_PATH}/RF24Mesh
sudo make install -B -C ${ROOT_PATH}/RF24Mesh
git clone https://github.com/tmrh20/RF24Gateway.git ${ROOT_PATH}/RF24Gateway
sudo make install -B -C ${ROOT_PATH}/RF24Gateway
