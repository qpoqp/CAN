import React, {useContext} from 'react'
import {StyleSheet, View} from 'react-native';
import {Text} from 'react-native-elements';
import {Context} from '../../context';
const MCStatus = () =>{
    const {state, dispatch} = useContext(Context);
    const styles = StyleSheet.create({
      container: {
      },
      text: {
        color: state.isConnected.ssid != '' ? '#43b581' : '#f04747', 
        textAlign: 'center',
        letterSpacing: 2,
      },
    });
    return (
      <View style={styles.container}>
        <Text style={styles.text}>
          {state.isConnected.ssid != '' ? 'Подключено' : 'Соединение разорвано'}
        </Text>
      </View>
    );
}




export default MCStatus;