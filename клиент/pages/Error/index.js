import React, {useContext} from 'react'
import {StyleSheet, View} from 'react-native';
import {Text} from 'react-native-elements';
import {Context} from '../../context';
const Error = () => {
  const {state, dispatch} = useContext(Context);
  return (
    <View style={styles.body}>
      <Text h1 style={styles.error}>
        {state.error.tittle}
      </Text>
      <Text h5 style={styles.error}>
        {state.error.body}
      </Text>
    </View>
  );
};
const styles = StyleSheet.create({
    body:{
        backgroundColor: 'black',
        flex: 1,
        alignItems: 'center',
        justifyContent: 'center'
    },
    error: {
        color: 'red',
    }
});
export default Error;