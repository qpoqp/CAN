import React, { useEffect, useContext } from 'react';
import {View, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';
import {Text } from 'react-native-elements';
import { Icon } from 'react-native-elements/dist/icons/Icon';
import AsyncStorage from '@react-native-async-storage/async-storage';
import WifiManager from "react-native-wifi-reborn";
import {Loading} from '../../components';
import {Context} from '../../context';

const saveUserSettings = async () =>{
    try {
      await AsyncStorage.setItem('@Settings:color', 'I like to save it.');
    } catch (error) {
      dispatch({
        type: 'error',
        payload: {tittle: '#AsyncStorage', body: 'Нет доступа'},
      });
    }
   }

const Settings = ({history}) =>{
    const {state, dispatch} = useContext(Context);
    useEffect(async ()=>{
        const data = await WifiManager.loadWifiList();
        await dispatch({type: 'wifi-list', payload: data});
    }, []);
    return (
      <ScrollView contentContainerStyle={styles.wifi_list}>
        {state.wifi_list.map((e, k) => (
          <TouchableOpacity
            key={k}
            onPress={() => history.push('/settings/' + e.SSID)}
            activeOpacity={0.3}
            style={styles.wifi_el}>
            <Icon
              size={30}
              name="wifi"
              color={state.isConnected.ssid === e.SSID ? '#43b581' : 'white'}
              iconStyle={{marginRight: 10, marginLeft: 10}}
            />
            <View style={styles.wifi_text_container}>
              <Text h4 style={{color: 'white'}}>
                {e.SSID}
              </Text>
              <Text h5>MAC:{e.BSSID}</Text>
            </View>
          </TouchableOpacity>
        ))}
      </ScrollView>
    );
}

const styles = StyleSheet.create({
  wifi_list: {
    padding: 10,
    zIndex: 0,
    alignItems: 'center',
    // backgroundColor: 'red',
  },
  wifi_el: {
    flexDirection: 'row',
    backgroundColor: '#121212',
    padding: 10,
    borderRadius: 15,
    marginBottom: 10,
  },
  wifi_text_container: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    flex: 1,
  },
});

export default Settings;