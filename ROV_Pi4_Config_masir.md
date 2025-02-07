# ROV Pi4 Config

# ROV LAMP + phpmyadmin

Ref: 
https://randomnerdtutorials.com/raspberry-pi-apache-mysql-php-lamp-server/


# ROV FTP

sudo apt-get install pure-ftpd -y

# ROV i2cTool

Link: https://zoomadmin.com/HowToInstall/UbuntuPackage/i2c-tools

sudo apt-get install -y i2c-tools

pip3 install smbus2


########################################
##Python Resful API Set up
########################################
Reference Link: https://www.jeremymorgan.com/tutorials/python-tutorials/how-to-rest-api-python/

Python3 Flask downgrade for incompatible issue:

pip3 install --force-reinstall sqlalchemy==1.4.48 flask-sqlalchemy==2.5.1
pip3 install --force-reinstall flask==2.2.5
pip3 install --force-reinstall Werkzeug==2.2.2

###After MariaDB Installed###

1. PYTHON / SQL Connection

=> install Python MySQL package
sudo apt-get install libmariadb client-dev libssl-dev python-mysqldb


For Python mysql connector
sudo pip3 install mysql-connector-python

=> Install curl command for post/get request for Linux
 sudo apt-get install curl

pip3 install --upgrade setuptools
pip3 install flask flask-jsonpify flask-sqlalchemy flask-restful flask-mysql flask_cors flask_mysql_connector


########################################
##ROV Database
########################################

CREATE TABLE IF NOT EXISTS `rov_float` (
  `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `ssc_id` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `timezone` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `p` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `p_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `d` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `d_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `t` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `t_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `a` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `a_unit` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci	;



