checklistdatfile = open("check.dat", "wb")

systimebytearray = bytearray([255] * 16)

checklistdatfile.write(systimebytearray)
checklistdatfile.write(u"dddddddddddddddddddddddddddddddddddd\0".encode("utf-16"))

checklistdatfile.write(systimebytearray)
checklistdatfile.write(u"The doc\0".encode("utf-16"))

checklistdatfile.write(systimebytearray)
checklistdatfile.write(u"The docEEEE\0".encode("utf-16"))


checklistdatfile.close()