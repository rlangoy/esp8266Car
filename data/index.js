/*
  Joustick code from:
  https://github.com/yoannmoinet/nipplejs
*/

var radius = 100;
var sampleJoystick = {
    zone: document.getElementById('joystick_base'), 
    mode: 'static',
    position: {
      left: '50%',
      top:   '175px' //'10%'
    },
    size: radius*2,
    color: 'black'
};

var joystick;
var position;
joystick = nipplejs.create(sampleJoystick);

/*
joystick.on('move', function(evt, data) 
{
  console.log('Move', data);
};

joystick.on('start end', function(evt, data) {};

 joystick.on('dir:up plain:up dir:left plain:left dir:down plain:down dir:right plain:right', 
  function(evt, data) {};


.on('pressure', function(evt, data) 
{
  position=data;
});
*/

joystick.on('start end', function(evt, data)
 {
  position = data; }).on('move', function(evt, data) {
  position = data;
  console.log('On Move', data.distance, 'rad', data.angle.radian);
}).on('dir:up plain:up dir:left plain:left dir:down' +
      'plain:down dir:right plain:right',
      function(evt, data) {        
  //position=data;
}
     ).on('pressure', function(evt, data) {
  position=data;

});




