import React, {cloneElement, useContext, useEffect, useState} from 'react';
import {View, ScrollView, StyleSheet, Alert, TouchableOpacity} from 'react-native';
import {Text, Icon} from 'react-native-elements';
import {Context} from '../../context';
import {Loading} from '../../components';
const Monitor = () =>{
  const {state, dispatch} = useContext(Context);
  const [time, setTime] = useState(Date.now() / 10);
  useEffect(()=>{
    //Переход в режим монитора
    const timer = setInterval(()=>{
      setTime(Math.floor(Date.now() / 1000));
    }, 1000);
    if (state.isConnected.ssid !== '')
      dispatch({
        type: 'TCPAction',
        payload: {id: '_MONITOR', type: '1', value: '0'},
      });
    return () => {
      clearInterval(timer);
      if (state.isConnected.ssid !== '')
        dispatch({
          type: 'TCPAction',
          payload: {id: '_MONITOR', type: '0', value: '0'},
        });
    };
  }, []);
  return state.monitor.length ? (
    <ScrollView contentContainerStyle={styles.table}>
      <View style={styles.table_row}>
        <View style={styles.table_header}>
          <Text style={styles.table_text}>CAN ID</Text>
        </View>
        {[...Array(8)].map((a, i) => (
          <View key={i} style={styles.table_header}>
            <Text style={styles.table_text}>Байт {i}</Text>
          </View>
        ))}
      </View>
      {state.monitor.map((e, i) => (
        <View key={i} style={styles.table_row}>
          <View style={styles.table_header}>
            <Text style={styles.table_text}>
              {e[0].value.toString(16).toUpperCase()}
            </Text>
          </View>
          {e.map(
            (z, j) =>
              j > 0 && ( //на 0 позиции находится CAN ID
                <View key={j} style={styles.table_header}>
                  <Text
                    style={{
                      ...styles.table_text,
                      backgroundColor: 'transparent',
                      fontWeight: 'normal',
                      color:
                        'rgb(255,' +
                        (time - z.timeStamp)*85+
                        ',' +
                        (time - z.timeStamp)*85 +
                        ')',
                    }}>
                    {z.value.toString(16).toUpperCase()}
                  </Text>
                </View>
              ),
          )}
        </View>
      ))}
    </ScrollView>
  ) : (
    <Loading />
  ); 
}
const styles = StyleSheet.create({
  table: {
    // flexGrow: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  table_row: {
    flex: 1,
    alignSelf: 'stretch',
    flexDirection: 'row',
    borderTopWidth: 1,
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(0,0,0,0.5)',
  },
  table_header: {
    flex: 1,
    alignSelf: 'stretch',
    borderRightColor: 'rgba(0,0,0,0.5)',
    borderRightWidth: 1,
  },
  table_text: {
    textAlign: 'center',
    backgroundColor: '#121212',
    padding: 3,
    fontWeight: 'bold',
    color: 'white',
    fontSize: 18,
  },
  notFound: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  }
});

export default Monitor;