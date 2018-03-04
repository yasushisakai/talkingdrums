# the talking drums

[project page](https://yasushisakai.github.io/talkingdrums/)



# auto start processing sketch

## 
change following file
```
~/.config/lxsession/LXDE-pi/autostart
```

and add

```
/usr/local/bin/processing-java --sketch=/home/pi/sketchbook/sketchname --run
```

the path to processing-java and sketchbook path should be changed accordingly
