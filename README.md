# EBB Sense
  Controlling a wearable display using a bitalino EDA sensor

### Software
- Arduino Code

### Serial Graph for Debugging
- Graph the data from the arduino code running over Serial port
- Scale using up and down keys

### Hardware

![Custom PCB](https://github.com/Noura/ebbsense/blob/master/images/beautifulPCB_simplified-01.jpg)

- Eagle schematic and board
- Current version is 2.1

### Shirt

![Custom PCB](https://github.com/Noura/ebbsense/blob/master/images/shirt_rippleeffect.jpg)

Each pinstripe had about 2-7Ω resistance and received about 450mA from a 3.7V LiPo battery via an Adafruit Powerboost 5V regulator, switching on (100% duty cycle) or off (0%) with a transistor controlled by Arduino pins. Each pinstripe consists of a single conductive thread. Snaps on the inside of the shirt provided removable but secure electrical connections to the thread endpoints. A Bitalino skin conductance sensor was placed on the back of the left shoulder of the shirt to hide the sensor from public view and secure it within the shirt. 

![Custom PCB](https://github.com/Noura/ebbsense/blob/master/images/electronics_diagram.jpg)

Fabric enclosures for electronics fit inside the shirt. (a) Skin conductance sensor with electrodes on colored paper for visibility. (b) Snaps connections to pinstripes. (c) Diagrammatic representation of pinstripes, which in actuality were sewn into shirt. (d) Snaps connections with FSRs for override on/off features. (e) Diagrammatic representation of FSRs, which in actuality were embedded in shirt seams. (f) PCB and battery, which were hidden in an interior shirt pocket.

---

### Team
- Noura Howell
- Laura Devendorf
- Rundong Tian
- Tomás Vega
