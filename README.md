## Heart Pendant
> A two part PCB, one used as a bracelet that senses heart pulses, and a pendant shaped as a heart that shows them

---

### What is Heart Pendant?
Its a small wearable made of two boards on a single PCB, joined by a break-away joint, The square half is a bracelet that measures the pulse with a heartbeat sensor.The heart-shaped half is a necklace pendant that shows each beat as a light animation moving smoothly around a ring of addressable LEDs. Once you snap the halves apart, each one works on its own and they talk to each other wirelessly.

### Why is Heart Pendant useful?
Its not really useful :), but its just fun and a reminder that each second your heart beats :) or something. I mean its just cool.

---

### How it works?
The bracelet's ESP32-C3 reads a heartbeat sensor module and detects each beat. It sends the beat to the pendant using the ESP-NOW protocol. Then the pendants ESP32 displays that heartbeat as a simple rotational animation using the addressable LEDs. Each half has its own LiPo battery, USB C port with a dedicated charge controller, and a 3v3 regulator, so both can be charged and used separately.

## So far
- [x] Schematic: ESP32 core, USB-C input, charger, regulator, power switch, battery sensing
- [ ] Pulse sensor and LED ring
- [ ] PCB layout
- [ ] Firmware
- [ ] Case and strap