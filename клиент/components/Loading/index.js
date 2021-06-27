import React, {Suspense} from 'react'
import {View, ActivityIndicator, StyleSheet, Text} from 'react-native'
const Loading = () =>{
    return (
      <View style={[styles.container]}>
        <ActivityIndicator size={70} color="#f04747" />
      </View>
    );
}
const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center'
  },
});
  
export default Loading;