//useReducer        -> Инициализация глобального хранилища "state" и функции "disptach" для его обновления.
//NativeRouter      -> Подключение библиотеки react-router
//SafeAreaProvider  -> Зависимость, необходимая для react-native-elements. Нормализует отображение на разных устройствах
//StatusBar hidden  -> Скрыает StatusBar
//container         -> Формирует dom дерево в стиле Header Body Footer
//Context.Provider  -> Пробрасывает хранилище и dispatch в child компоненты (страницы роутинга)
//Route             -> Отрисовывает необходимый компонент, в зависимости от location
//Link              -> JSX компонент позволяющий перемещаться между роутами, аналог программного history.replace
//StyleSheet.create -> Таблица стилей
import React, {useEffect, useReducer, useRef} from 'react';
import {
  StyleSheet,
  Text,
  View,
  StatusBar,
  PermissionsAndroid,
  Alert,
} from 'react-native';
import {NativeRouter, Route, Link, BackButton} from 'react-router-native';
import {SafeAreaProvider} from 'react-native-safe-area-context';
import {Context} from './context';
import reducer from './reducer';
import {Home, Settings, Monitor, Error, CurrentDevice} from './pages';
import {MCStatus, Menu, CANFilter} from './components';
import AsyncStorage from '@react-native-async-storage/async-storage';
import WifiManager from "react-native-wifi-reborn";
var net = require('react-native-tcp');

const App = () => {
  //Статическая ссылка на TCP соединение. При очередном rerender не будет создаваться новое TCP соединение
  const [state, dispatch] = useReducer(reducer, {
    error: false,
    isConnected: {ssid: ''},
    pages: [
      {title: 'Главная', url: '/', ico: 'home'},
      {title: 'Монитор', url: '/monitor', ico: 'search'},
      {title: 'Настройки', url: '/settings', ico: 'cog'},
    ],
    activePage: 0,
    files: null,
    monitor: [],
    wifi_list: [],
    file_list: [],
    TCPAction: {id: '0', type: '0', value: '0'},
    isSimulation: false,
    isRecord: false,
  });
  useEffect(async () => {
    requireWiFiPermission();
    initAsyncStorage();
    return () => {
      //ComponentWillUnMount
    };
  }, []);
  const requireWiFiPermission = async () => {
    const granted = await PermissionsAndroid.request(
      PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
    );
    dispatch({
      type: 'error',
      payload: granted === PermissionsAndroid.RESULTS.DENIED && {
        tittle: '#Wi-Fi',
        body: 'Нет доступа',
      },
    });
  };
  const initAsyncStorage = async () => {
    try {
      const value = await AsyncStorage.getItem('@Settings:color');
      console.log(value);
    } catch (error) {
      dispatch({
        type: 'error',
        payload: {tittle: '#AsyncStorage', body: 'Нет доступа'},
      });
    }
  };

  const TCP = useRef();
  const monitorRef = useRef();
  const fileListRef = useRef();
  monitorRef.current = state.monitor;
  fileListRef.current = state.file_list;


  useEffect(() => {
    const {id, type, value} = state.TCPAction;
    console.log(id + " " + type + " " + value);
    if(state.isConnected.ssid !== '' || state.TCPAction.id === '_CONNECT'){
      switch (id) {
        case '_CONNECT': {
          TCP.current = {connection: null, onData: null, onClose: null, onError: null};
          //Инициализирует TCP соединение с микроконтроллером и устанавливает скорость шины CAN
          TCP.current.connection = net.createConnection(
            {
              port: 5555,
              host: '192.168.0.1',
              tls: false,
            },
            () => {
              //В случае успешного соединения
              TCP.current.onData = TCP.current.connection.on('data', data => {
                if (data.length === 3) {
                  //Пришел ответ от сервера
                  Alert.alert('Сервер', data[2] === 49 ? 'Успешно' : 'Ошибка');
                }
                if(data.length >= 10){//Пришло название файла, недостающие байты заполняются ascii = 32 на стороне сервера
                  const filename = [];
                  data.map((e, i) => filename[i] = String.fromCharCode(e));
                  dispatch({
                    type: 'FILEPacket',
                    payload: [...fileListRef.current, filename.join('')],
                  });
                }
                if (data.length === 9) {
                  //Пришел кадр CAN, всегда = 8bytes + 1 byte на идентификатор устройства
                  //Нулевой элемент массива - Идентификатор устройства
                  //Остальные элементы массива - 0...7 байты данных
                  var exist = false;
                  for(let i = 0; i < monitorRef.current.length; i++){
                    if(monitorRef.current[i][0].value === data[0]){
                      exist = true;
                      monitorRef.current[i].map((byte, z) => {
                        //проход по конкретным байтам
                        if (byte.value !== data[z]) {
                          //хранящийся байт отличается от нового
                          byte.value = data[z];
                          byte.timeStamp = Math.floor(Date.now() / 1000); //у конкретного байта устанавливается временная метка последнего изменения   Math.floor(Date.now() / 1000)
                        }
                      });
                      break;
                    }
                  }
                  // monitorRef.current.map(e => {
                  //   //проход по уже полученным пакетам
                  //   if (e[0].value === data[0]) {
                  //     //Если в полученных сушествует пакет с идентификатором нового
                  //     exist = true;
                  //     e.map((byte, i) => {
                  //       //проход по конкретным байтам
                  //       if (byte.value !== data[i]) {
                  //         //хранящийся байт отличается от нового
                  //         byte.value = data[i]; //+1 т.к. нумерация нового с 1 (в 0 позиции идентификатор устройства)
                  //         byte.timeStamp = Math.floor(Date.now() / 1000); //у конкретного байта устанавливается временная метка последнего изменения   Math.floor(Date.now() / 1000)
                  //       }
                  //     });
                  //   }
                  // });
                  if (exist === false) {
                    //Идентификатор нового кадра не найден среди имеющихся
                    monitorRef.current.push([]);
                    data.map(e =>
                      monitorRef.current[monitorRef.current.length - 1].push({
                        timeStamp: Math.floor(Date.now() / 1000),
                        value: e,
                      }),
                    );
                  }
                  dispatch({type: 'CANPacket', payload: monitorRef.current});
                }
              });
              TCP.current.onClose = TCP.current.connection.on('close', () => {
                  dispatch({type: 'wifi-status', payload: {ssid: ''}});
                  WifiManager.disconnect();
              });
              TCP.current.onError = TCP.current.connection.on('error', (e)=>{
                console.log(e);
              })
              // _CONNECT = 0
              // type = 1 - подключение / 0 - отключение
              //value = скорость шины CAN =  '0' '1' '2' '3' '4' '5'... => смотреть combobox
              TCP.current.connection.write('0' + type + value);
            },
          );
          break;
        }
        //Управление шиной CAN, начать симуляцию / завершить
        case '_CAN': {
          TCP.current.connection.write('1' + type + value);
          break;
        }
        //Начать запись показаний с шины в файл с названием value, max длинна value = 14 байт
        case '_RECORD': {
          TCP.current.connection.write('2' + type + value);
          break;
        }
        case '_MONITOR': {
          TCP.current.connection.write('3' + type + value);
          break;
        }
        case '_FILE': {
          if(type === '0'){//Если тип - получить список файлов
            dispatch({type: 'FILEPacket', payload: []});//очищает старый список файлов 
          }
          TCP.current.connection.write('4' + type + value);
          break;
        }
        case '_SETTINGS': {
          TCP.current.connection.write('5' + value);
          break;
        }
        case '_DISCONNECT': {
          TCP.current.connection.destroy();
          TCP.current.connection = null;
          TCP.current.onData = null;
          TCP.current.onClose = null;
          break;
        }
      }
    }else{
      Alert.alert("Ошибка", "соединение не установлено");
    }
  }, [state.TCPAction]);

  return (
    <Context.Provider value={{dispatch, state}}>
      {!state.error ? (
        <NativeRouter>
          <SafeAreaProvider>
            <StatusBar hidden />
            <View style={styles.container}>
              <View style={styles.header}>
                <MCStatus />
              </View>
              <View style={styles.body}>
                <Route exact path="/" component={Home} />
                <Route exact path="/settings" component={Settings} />
                <Route path="/monitor" component={Monitor} />
                <Route path="/settings/:device" component={CurrentDevice} />
              </View>
              <View style={styles.footer}>
                <Menu />
              </View>
            </View>
          </SafeAreaProvider>
        </NativeRouter>
      ) : (
        <Error />
      )}
    </Context.Provider>
  );
};
const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#1b1b1b',
  },
  header: {
    backgroundColor: '#121212',
  },
  body: {
    flex: 1,
  },
  footer: {
    backgroundColor: '#121212',
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
});
export default App;
