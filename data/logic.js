let counter=1;
setTextColor();

const d = new Date();
let TimeVal = d.getHours()+":"+d.getMinutes()+":"+d.getSeconds();
let DateVal = d.getDate()+","+d.getMonth()+","+d.getFullYear();
console.log(DateVal);


function CurrentTime(){
  //console.log(document.getElementsByClassName("dungen_value time")[0]);
  let num = String(d.getHours()).length;
if (num === 1) {
   HoursVal="0"+d.getHours();
} 
else {
  HoursVal=d.getHours();
}
  let curTime=HoursVal+":"+d.getMinutes()+":"+d.getSeconds();
  document.getElementsByClassName("dungen_value time")[0].innerHTML=curTime;
  //return d.getHours()+":"+d.getMinutes()+":"+d.getSeconds()
}

function CurrentDate(){
  //console.log(document.getElementsByClassName("dungen_value time")[0]);
  let DateVal = d.getDate()+","+d.getMonth()+","+d.getFullYear();
  document.getElementsByClassName("dungen_value date")[0].innerHTML=DateVal;
  //return d.getHours()+":"+d.getMinutes()+":"+d.getSeconds()
}

function autoDarkMode(){
  let Hours = d.getHours();
  //let Hours = 6;
  if(Hours>=22 || Hours<=5)
  {
    toggleDarkMode();
  }
}

function toggleDarkMode() {
  var element = document.body;
  element.classList.toggle("dark-mode");
  counter+=1;
  if (counter%2==0)
  {
    document.getElementsByClassName("dark-mode-button")[0].setAttribute("src","on_bubl.png");
  }
  else
  {
    document.getElementsByClassName("dark-mode-button")[0].setAttribute("src","off_bubl.png");
  }
  document.getElementsByClassName("measured_value indoor_temp")[0].style.color = "blue";
}

function setTextColor() {
  if (document.getElementsByClassName("text  indoor_temp")[0]){
  document.getElementsByClassName("text  indoor_temp")[0].style.color="cyan";
}
}