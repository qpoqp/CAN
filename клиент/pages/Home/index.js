import React, {useContext, useEffect, useRef, useState} from 'react';
import {
  View,
  ScrollView,
  StyleSheet,
  Alert,
  TouchableOpacity,
  Pressable,
} from 'react-native';
import {
  Button,
  Input,
  CheckBox,
  Divider,
  Text,
  Slider,
} from 'react-native-elements';
import {Context} from '../../context';
import {Icon} from 'react-native-elements/dist/icons/Icon';
const Home = () => {
  const {state, dispatch} = useContext(Context);
  const [home, setHome] = useState({filename: '', maxSize: 1, deviceCount: 1});
  const homeRef = useRef();
  homeRef.current = home;
  const {ssid} = state.isConnected;
  const homeController = (_id, _type, _value) => {
    if (state.isConnected.ssid !== '') {
      return dispatch({
        type: 'TCPAction',
        payload: {id: _id, type: _type ? '0' : '1', value: _value},
      });
    }
  };
  return (
    <ScrollView contentContainerStyle={styles.home}>
      <Input
        placeholder="Название файла"
        maxLength={13}
        placeholderTextColor="#858585"
        color="#858585"
        value={home.filename}
        onChangeText={e => setHome({...homeRef.current, filename: e})}
        leftIcon={
          <Icon name="book" type="font-awesome" size={24} color="#858585" />
        }
      />
      <View
        style={{
          flex: 1,
          alignItems: 'stretch',
          justifyContent: 'center',
          alignSelf: 'stretch',
          marginRight: 10,
          marginLeft: 10,
        }}>
        <Text style={{color: 'white'}}>
          Максимальный размер файла: {home.maxSize} KB ~{' '}
          {new Date(home.maxSize * 3.4 * 1000).toISOString().substr(11, 8)}
        </Text>
        <Slider
          trackStyle={{height: 2}}
          thumbStyle={{height: 20, width: 6, backgroundColor: '#f04747'}}
          value={home.maxSize}
          onValueChange={e => setHome({...homeRef.current, maxSize: e})}
          maximumValue={127}
          minimumValue={1}
          step={1}
        />
        <Text style={{color: 'white'}}>
          Количество устройств в симуляции: {home.deviceCount}
        </Text>
        <Slider
          trackStyle={{height: 2}}
          thumbStyle={{height: 20, width: 6, backgroundColor: '#f04747'}}
          value={home.deviceCount}
          onValueChange={e => setHome({...homeRef.current, deviceCount: e})}
          maximumValue={127}
          minimumValue={1}
          step={1}
        />
      </View>
      <TouchableOpacity
        disabled={ssid === ''}
        activeOpacity={0.3}
        style={styles.btn_container}
        onPress={() => {
          dispatch({type: 'isSimulation-btn'});
          homeController(
            '_CAN',
            state.isSimulation,
            String.fromCharCode(home.deviceCount),
          ); //отправка кол-ва девайсов для симуляции
        }}>
        <Text
          style={{
            ...styles.btn,
            backgroundColor: ssid !== '' && '#121212',
          }}>
          {state.isSimulation ? 'Остановить симуляцию' : 'Начать симуляцию'}
        </Text>
      </TouchableOpacity>
      <TouchableOpacity
        disabled={ssid === ''}
        activeOpacity={0.3}
        style={styles.btn_container}
        onPress={() => {
          dispatch({type: 'isRecord-btn'});
          homeController(
            '_RECORD',
            state.isRecord,
            String.fromCharCode(home.maxSize) + home.filename,
          );
        }}>
        <Text
          style={{
            ...styles.btn,
            backgroundColor: ssid !== '' && '#121212',
          }}>
          {state.isRecord ? 'Остановить запись' : 'Начать запись'}
        </Text>
      </TouchableOpacity>
      <TouchableOpacity
        disabled={ssid === ''}
        activeOpacity={0.3}
        style={styles.btn_container}
        onPress={() => homeController('_FILE', '0', '0')}>
        <Text
          style={{
            ...styles.btn,
            backgroundColor: ssid !== '' && '#121212',
          }}>
          Обновить список файлов
        </Text>
      </TouchableOpacity>
      {ssid === '' && <Divider style={styles.divider} /> }
      <View style={styles.file_notification}>
        {state.file_list.length > 1 ? (
          state.file_list.map(
            (e, i) =>
              i > 0 && (
                <View key={i} style={styles.currentFile}>
                  <Text
                    key={i}
                    h5
                    style={{
                      color: 'white',
                      textAlign: 'center',
                    }}>
                    {e}
                  </Text>
                  <Text
                    onPress={() => homeController('_FILE', false, e)}
                    style={{
                      paddingRight: 10,
                      paddingLeft: 10,
                      color: '#f04747',
                      borderRadius: 15,
                    }}>
                    Удалить
                  </Text>
                </View>
              ),
          )
        ) : (
          <Text h5 style={{color: 'white', marginTop: 10}}>
            Список файлов пуст...
          </Text>
        )}
      </View>
    </ScrollView>
  );
};
const styles = StyleSheet.create({
  home: {
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
  divider: {
    backgroundColor: '#858585',
    alignSelf: 'stretch',
    marginBottom: 20,
    marginRight: 10,
    marginLeft: 10,
  },

  file_notification: {
    alignSelf: 'stretch',
    alignItems: 'center',
    marginBottom: 20,
  },

  currentFile: {
    alignSelf: 'stretch',
    marginRight: 30,
    marginLeft: 30,
    marginBottom: 20,
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },

  table: {
    // flexGrow: 1,
    alignSelf: 'stretch',
    alignItems: 'center',
    justifyContent: 'center',
    margin: 10,
    marginRight: 0,
    borderBottomColor: 'rgba(0,0,0,0.5)',
  },
  table_row: {
    flex: 1,
    alignSelf: 'stretch',
    flexDirection: 'row',
  },
  table_header: {
    flex: 1,
    alignSelf: 'stretch',
    borderRightColor: 'rgba(0,0,0,0.5)',
    marginRight: 10,
  },
  table_text: {
    textAlign: 'center',
    backgroundColor: '#121212',
    padding: 3,
    color: 'white',
    fontSize: 17,
  },
});
export default Home;
