import React, {useState, useContext} from 'react'
import {StyleSheet, ScrollView, Dimensions, Alert, TouchableOpacity} from 'react-native'
import {Text, Input, Button, CheckBox,} from 'react-native-elements'
import { Icon } from 'react-native-elements/dist/icons/Icon';
import {Picker} from '@react-native-picker/picker';
import WifiManager from "react-native-wifi-reborn";
import {Context} from '../../context';

const CurrentDevice = ({match}) =>{
    const {state, dispatch} = useContext(Context);
    const [wifiData, setWifiData] = useState({
      ssid: match.params.device,
      pass: '',
      CAN: '0',
      isLoading: false,
      settings: {ssid: '', pass: ''}
    });
    const connectToSSID = ()=>{
      setWifiData({...wifiData, isLoading: true});
      if (state.isConnected.ssid === match.params.device) {
        WifiManager.disconnect();
        dispatch({type: 'TCPAction', payload: {id: '_DISCONNECT'}});
        dispatch({type: 'wifi-status', payload: {ssid: ''}});
        setWifiData({...wifiData, isLoading: false});
      }else{
        WifiManager.connectToProtectedSSID(
          wifiData.ssid,
          wifiData.pass,
          false,
        ).then(
          () => {
            dispatch({type: 'wifi-status', payload: {ssid: match.params.device}});
            dispatch({type: 'TCPAction', payload: {id: '_CONNECT', type: '1', value: wifiData.CAN}});//wifiData.CAN - скорость шины CAN
            setWifiData({...wifiData, isLoading: false});
          },
          () => {
            Alert.alert('Ошибка', 'Не верный пароль...');
            setWifiData({...wifiData, isLoading: false});
          },
        );
      } 
    }
    const setSettings = () =>{
      const {ssid, pass} = wifiData.settings;
      if (
        ssid.length > 0 &&
        ssid.length <= 10 &&
        pass.length > 7 &&
        pass.length <= 19
      )
        dispatch({
          type: 'TCPAction',
          payload: {
            id: '_SETTINGS',
            value: wifiData.settings.ssid + '|' + wifiData.settings.pass,
          },
        });
      else
        Alert.alert(
          'Ошибка',
          'Длинна SSID ∈  [1;10]\n\nДлинна нового пароля ∈  [8;19]',
        );
    }
    return (
      <ScrollView contentContainerStyle={styles.settings}>
        <Input
          placeholder="SSID"
          placeholderTextColor="#858585"
          color="#858585"
          leftIcon={<Icon name="wifi" size={24} color="#858585" />}
          value={wifiData.ssid}
          disabled
        />
        <Input
          disabled={wifiData.isLoading}
          placeholder="Пароль"
          placeholderTextColor="#858585"
          color="#858585"
          leftIcon={<Icon name="lock" size={24} color="#858585" />}
          secureTextEntry={true}
          value={wifiData.pass}
          onChangeText={e => setWifiData({...wifiData, pass: e})}
        />
        <Picker
          enabled={true}
          disabled={wifiData.isLoading}
          dropdownIconColor="#858585"
          style={{alignSelf: 'stretch', color: '#858585'}}
          selectedValue={wifiData.CAN}
          onValueChange={(itemValue, itemIndex) =>
            setWifiData({...wifiData, CAN: itemValue})
          }>
          <Picker.Item label="25KBIT/S" value="0" />
          <Picker.Item label="50KBIT/S" value="1" />
          <Picker.Item label="100KBIT/S" value="2" />
          <Picker.Item label="125KBIT/S" value="3" />
          <Picker.Item label="250KBIT/S" value="4" />
          <Picker.Item label="500KBIT/S" value="5" />
          <Picker.Item label="800KBIT/S" value="6" />
          <Picker.Item label="1MBIT/S" value="7" />
        </Picker>
        <Button
          title={
            state.isConnected.ssid === match.params.device
              ? 'Отключиться'
              : 'Подключиться'
          }
          onPress={() => connectToSSID()}
          containerStyle={{
            alignSelf: 'stretch',
            marginLeft: 10,
            marginRight: 10,
          }}
          disabledStyle={{backgroundColor: 'transparent'}}
          loading={wifiData.isLoading}
          disabled={wifiData.isLoading}
          buttonStyle={{
            backgroundColor: '#121212',
            padding: 13,
            marginBottom: 10,
          }}
        />
        <Input
          placeholder="Новый SSID"
          placeholderTextColor="#858585"
          color="#858585"
          leftIcon={<Icon name="wifi" size={24} color="#858585" />}
          value={wifiData.settings.ssid}
          onChangeText={e => {
            let data = {...wifiData};
            data.settings.ssid = e;
            setWifiData(data);
          }}
        />
        <Input
          placeholder="Новый пароль"
          placeholderTextColor="#858585"
          color="#858585"
          leftIcon={<Icon name="lock" size={24} color="#858585" />}
          secureTextEntry={true}
          value={wifiData.settings.pass}
          onChangeText={e => {
            let data = {...wifiData};
            data.settings.pass = e;
            setWifiData(data);
          }}
        />

        <TouchableOpacity
          disabled={state.isConnected.ssid === ''}
          activeOpacity={0.3}
          style={styles.btn_container}
          onPress={() => setSettings()}>
          <Text
            style={{
              ...styles.btn,
              backgroundColor: state.isConnected.ssid !== '' && '#121212',
            }}>
            Сохранить
          </Text>
        </TouchableOpacity>
      </ScrollView>
    );
}
const styles = StyleSheet.create({
    settings:{
        padding: 10,
        alignItems: 'flex-end',
    },
    btn_container: {
      alignSelf: 'stretch',
      marginLeft: 10,
      marginRight: 10,
      marginBottom: 10,
    },
    btn: {
      padding: 13,
      color: 'white',
      textAlign: 'center',
      fontWeight: 'bold',
      fontSize: 16,
    },
});
export default CurrentDevice;
// const styles = StyleSheet.create({
{/* <Text h1>{match.params.device}</Text> */}
// });