if [ $# != 1 ] ; then
	 echo "$0 <SSID>"
	  exit
  fi
  iwconfig wlan0 txpower 10
  $rmt wpa_cli -iwlan0 disconnect
  $rmt wpa_cli -iwlan0 remove_network all
  $rmt wpa_cli -iwlan0 add_network
  $rmt wpa_cli -iwlan0 set_network 0 frequency 5180
  $rmt wpa_cli -iwlan0 set_network 0 mode 1
  if [ ! -n "$rmt" ] ; then
	  $rmt wpa_cli -iwlan0 set_network 0 ssid \"$1\"
  else
	  $rmt wpa_cli -iwlan0 set_network 0 ssid '\"'$1'\"'
  fi
  $rmt wpa_cli -iwlan0 set_network 0 auth_alg OPEN
  $rmt wpa_cli -iwlan0 set_network 0 key_mgmt NONE
  $rmt wpa_cli -iwlan0 set_network 0 scan_ssid 1
  $rmt wpa_cli -iwlan0 select_network 0
  $rmt wpa_cli -iwlan0 enable_network 0
  $rmt wpa_cli -iwlan0 status
