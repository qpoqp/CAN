import React, {useContext} from 'react'
import {Text, StyleSheet, View} from 'react-native';
import {Context} from '../../context';
import {Link} from 'react-router-native';
import { Icon } from 'react-native-elements';


const Menu = () =>{
    const {state, dispatch} = useContext(Context);
    return state.pages.map((e, k) => (
      <Link
        onPress={() => dispatch({type: 'select-menu-item', payload: k})}
        key={k}
        to={e.url}
        underlayColor="transparrent"
        style={{padding: 3}}>
        <View>
          <Icon
            name={e.ico}
            type="font-awesome-5"
            size={22}
            color={state.activePage === k ? 'white' : '#858585'}
          />
          <Text
            style={{
              ...styles.bar_el,
              color: state.activePage === k ? 'white' : '#858585',
            }}>
            {e.title}
          </Text>
        </View>
      </Link>
    ));
}
const styles = StyleSheet.create({
  bar_el: {
    letterSpacing: 1.3,
    borderRadius: 5, 
    fontSize: 12
  },
});
export default Menu;