# Smart_Plant_Monitoring_systems
A simple Iot project that monitors the environment around the plant using sensors and actuators
# Smart Plant Monitoring System

> **IoT + FreeRTOS** — ESP32-based plant health monitor with DHT22 (temperature & humidity) and soil moisture sensing; automated watering via a relay; real-time telemetry via MQTT to a cloud dashboard.

---

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Hardware Required](#hardware-required)
* [Wiring](#wiring)
*

---

## Overview

A beginner-to-intermediate IoT project demonstrating real hardware, production-style code layout, and cloud telemetry. Intended for portfolio use: clear README, wiring diagrams, code comments, and a short demo video make this attractive to recruiters.

## Features

* Multi-sensor monitoring: temperature, humidity, soil moisture
* FreeRTOS task-based architecture (sensor tasks, pump control task, MQTT publisher)
* Automated watering using a relay and mini pump
* Telemetry over MQTT to a cloud dashboard (ThingsBoard by default)
* Config samples to avoid committing secrets

## Hardware Required

* ESP32 development board
* DHT22 sensor (or DHT11 with minor code changes)
* Capacitive soil moisture sensor (recommended) or YL-69
* Relay module (suitable for your pump voltage/current)
* Mini water pump (and suitable power supply)
* Jumper wires, breadboard, optional resistor for DHT data line

## Wiring

**DHT22**

* VCC → 3.3V
* GND → GND
* DATA → GPIO4 (with 4.7k–10k pull-up if needed)

**Soil Sensor (analog)**

* VCC → 3.3V
* GND → GND
* A0 → GPIO36 (ADC input)

**Relay & Pump**

* Relay IN → GPIO18
* Relay VCC → 3.3V/5V (module dependent)
* Relay GND → GND
* Pump +ve → Relay NO
* Pump -ve → External supply GND

> ⚠ Safety: Use a suitable external power supply for the pump. Do not power motors directly from the ESP32.



