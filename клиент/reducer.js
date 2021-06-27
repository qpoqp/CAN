export default function (state, action){
    switch (action.type) {
      case 'error': {
        return {...state, error: action.payload};
      }
      case 'select-menu-item':{
        return {...state, activePage: action.payload};
      }
      case 'wifi-list': {
        return {...state, wifi_list: action.payload};
      }
      case 'wifi-status':{
        return {...state, isConnected: action.payload};
      }
      case 'TCPAction':{
        return {...state, monitor: [], TCPAction: action.payload};
      }
      case 'CANPacket':{
        return {...state, monitor: action.payload};
      }
      case 'FILEPacket':{
        return {...state, file_list: action.payload};
      }
      case 'isSimulation-btn':{
        return {...state, isSimulation: !state.isSimulation};
      }
      case 'isRecord-btn':{
        return {...state, isRecord: !state.isRecord};
      }
      default:
        return state;
    }
}