#!/usr/bin/env python

import os, sys, getopt, bz2, time, datetime, re
from struct import *

PDBHeaderStructString='>32shhLLLlll4s4sllH'
PDBHeaderStructLength=calcsize(PDBHeaderStructString)
padding='\0\0'

enc = sys.getfilesystemencoding()
enc = 'gbk' if enc == 'UTF-8' else enc
  
class ZDic:
    def __init__(self,compressFlag=2,recordSize=0x4000):
        self.index=''
        self.byteLen=[]
        self.lenSec=''
        self.creatorID='Kdic'
        self.pdbType='Dict'
        self.maxWordLen=32
        self.recordSize=recordSize        
        self.compressFlag=compressFlag

    def trim(self,s):
        s = s.decode('utf-8').encode(enc,'replace').replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">")\
                .replace("&apos;", "'").replace("&quot;", '"').replace("&nbsp;", " ")
        s = re.compile('(</?(p|font|br|tr|td|table|div|span|ref|small).*?>)|(\[\[[a-z]{2,3}(-[a-z]*?)?:[^\]]*?\]\])|(<!--.*?-->)',\
                        re.I|re.DOTALL).sub('',s)
        return s

    def ste(self,s):       
        s = s.replace('<hr>','//STEHORIZONTALLINE//\n')\
                .replace('<center>','//STECENTERALIGN//').replace('</center>','\n')\
                .replace('<u>',"//STEHYPERLINK=").replace('</u>',"\1//")\
                .replace('<i>',"//STEBOLDFONT//").replace('</i>',"//STESTDFONT//")\
                .replace('<b>',"//STEBOLDFONT//").replace('</b>',"//STESTDFONT//")\
                .replace('<big>',"//STEBOLDFONT//").replace('</big>',"//STESTDFONT//")       
        s = re.sub('\[\[([^\[/]*?)\]\]',"//STEHYPERLINK=\g<1>\1//", s)        
        s = re.sub('\n{3,}','\n\n',s).replace('\n',r'\n')
        return s

    def unste(self,s):    
        return s.replace("//STEBOLDFONT//", '<b>').replace("//STESTDFONT//", '</b>')\
                .replace("//STEHYPERLINK=", '<u>').replace("\1//", '</u>')
    
    def fromPDB(self, path):
        "pdb=>byteList"
        f = open(path, 'rb')
        self.header = f.read(PDBHeaderStructLength)
        self.pdbName = self.header[:32].rstrip('\0')
        self.bnum = unpack('>l',self.header[-4:])[0]
        startOffset = unpack('>L', f.read(4))[0]
        self.lines = {}
        for i in range(self.bnum - 1):
            f.seek(PDBHeaderStructLength + (i + 1) * 8)
            endOffset = unpack('>L',f.read(4))[0]
            f.seek(startOffset)
            if i == 0:
                f.seek(7, 1)
                self.compressFlag = ord(f.read(1))
            else:
                s = f.read(endOffset - startOffset)
                if self.compressFlag == 2:
                    s = s.decode('zlib')
                for line in s.rstrip().split('\n'):
                    k, v = line.split('\t', 1)
                    self.lines[k] = v                
            startOffset = endOffset
        f.seek(startOffset)
        s = f.read()
        if self.compressFlag == 2:
            s = s.decode('zlib')
        for line in s.rstrip().split('\n'):
            k, v = line.split('\t')
            self.lines[k] = v       
        f.close()

    def fromPath(self, path):
        files = []        
        if os.path.isdir(path):
            files = glob.glob(os.path.join(path, '*.*'))
        else:
            files = [path] 
        self.lines = {}
        for path in files:
            ext=os.path.splitext(path)[1].lower()
            if ext in ['.xml', '.bz2']:#wiki xml
                f=bz2.BZ2File(path) if ext=='.bz2' else open(path)
                block=1024*32
                s=f.read(block)
                tmp=''
                while 1:  
                    try:
                        a=s.index('<title>')
                        b=s.index('</title>',a)
                        c=s.index('<text xml:space="preserve">',b)
                        d=s.index('</text>',c)
                        word,mean=self.trim(s[a+7:b]),self.trim(s[c+27:d])
                        if word in self.lines and mean[:9].lower()=='#redirect':
                            pass
                        else:
                            self.lines[word]=self.ste(mean)
                        s=s[d:]
                    except:
                        tmp=f.read(block)
                        if tmp:
                            s+=tmp
                        else:
                            break
                f.close()
            elif ext == '.pdb': #PDB dic
                self.fromPDB(path) 
            else: #TXT dic
                f=open(path,'rU')
                for i in f:
                    i = i.rstrip().replace(' /// ', '\t', 1)
                    if '\t' in i:
                        word, mean = i.split('\t', 1)
                        self.lines[word] = self.ste(mean)               

    def resizeBlock(self):
        tmp=''
        first=''
        f=open('tmp.pdb','wb')
        if '' not in self.lines:
            self.lines['']=self.ste('<b>Welcome to //STEPURPLEFONT//%s//STECURRENTFONT//!</b>\n<b>Words count:</b> //STEBLUEFONT//%d//STECURRENTFONT//\n<b>Made time:</b> //STEREDFONT//%s'%(self.pdbName,len(self.lines),time.strftime("%Y.%m.%d")))
        for word in sorted(self.lines.keys()):
            line = "%s\t%s\n"%(word,self.lines[word])
            if first=='':
                first=tmp
            if len(tmp+line)<self.recordSize:
                tmp+=line
            else:
                if tmp:
                    self.compress(tmp,first.split('\t',1)[0],tmp.rsplit('\n',2)[-2].split('\t',1)[0],f)
                    tmp=''
                if len(line)>=self.recordSize:
                    index,line=line.split('\t',1)
                    i=0
                    while line:
                        tmpindex=i and "%s %d"%(index,i) or index
                        blen=self.recordSize-len(tmpindex)-3
                        if blen>len(line):
                            tmpline=line
                            line=''
                        else:
                            if '//STE' in line[blen-40:blen+4]: #split form //STE
                                steindex=line[blen-40:blen+4].index('//STE')
                                blen=blen-40+steindex
                            elif '\\n' == line[blen-1:blen+1]: # split from \n
                                blen=blen-1
                            tmpline=line[:blen]
                            line=line[blen:]
                            try:
                                tmpline.decode('gbk')
                            except:
                                line=tmpline[-1]+line
                                tmpline=tmpline[:-1]                           
                        self.compress("%s\t%s\n"%(tmpindex,tmpline),tmpindex,tmpindex,f)
                        i+=1
                else:
                    tmp=line
                first=''
        if tmp:
            self.compress(tmp,first.split('\t',1)[0],line.rsplit('\n',2)[-2].split('\t',1)[0],f)
        f.close()
            
    def compress(self,block,first,last,f):
        block=block.encode('zlib')
        f.write(block)
        l=len(block)
        self.byteLen.append(l)
        self.lenSec+=pack('>H',l)
        self.index+="%s\0%s\0"%(first[:self.maxWordLen],last[:self.maxWordLen])         
            
    def packPalmDate(self, variable=None):
        PILOT_TIME_DELTA = 2082844800L        
        variable = variable or datetime.datetime.now()
        return int(time.mktime(variable.timetuple())+PILOT_TIME_DELTA)
        
    def packPDBHeader(self):
        return pack(PDBHeaderStructString,
                    self.pdbName,0,0,
                    self.packPalmDate(),0,0,0,0,0,
                    self.pdbType,self.creatorID,0,0,self.bnum
                )

    def toPDB(self,path):
        self.bnum=len(self.byteLen)
        self.index=pack('>I2H8x',1,self.bnum,2)+self.lenSec+ self.index
        del self.lenSec     
        #record index   
        self.byteLen.insert(0,len(self.index))
        self.bnum+=1
        towrite=self.packPDBHeader()
        offset=PDBHeaderStructLength + 8 * self.bnum +len(padding)
        uid=0x406F8000      
        for i in self.byteLen:
            towrite+=pack('>2L', offset, uid) 
            offset+=i
            uid+=1
        del offset,uid,i,self.byteLen
        towrite+=padding
        f=open(path,'wb')
        #header and index
        f.write(towrite+self.index)
        del towrite,self.index
        block=1024*1024*10
        tmp=open('tmp.pdb','rb')
        while 1:
            cont=tmp.read(block)
            if cont:
                f.write(cont)
            else:
                break
        tmp.close()
        f.close()
        os.remove('tmp.pdb')     

    def p2t(self, path, patho):
        "pdb=>byteList"
        f = open(path, 'rb')
        
        f.seek(PDBHeaderStructLength - 4)
        self.bnum= unpack('>l',f.read(4))[0]
        firstOffset = unpack('>L', f.read(4))[0]
        f.seek(4, 1)
        startOffset = unpack('>L', f.read(4))[0]
        f.seek(firstOffset + 7)
        self.compressFlag = ord(f.read(1))
        if self.compressFlag == 1:
            f.close()
            os.system('DeKDic.exe %s' % path)
            os.system('ren %s.tab %s' %(path, patho))
        else:
            t = open(patho, 'wb')
            for i in range(1, self.bnum - 1):
                f.seek(PDBHeaderStructLength + (i + 1) * 8)
                endOffset = unpack('>L',f.read(4))[0]
                f.seek(startOffset)
                t.write(self.unste(f.read(endOffset - startOffset).decode('zlib')))
                startOffset = endOffset
            f.seek(startOffset)
            t.write(self.unste(f.read().decode('zlib')))
            t.close()
            f.close()

def log(msg):
    print '[%s]%s'%(time.strftime('%X'), msg)
    
if __name__ == '__main__':    
    opts, argv = getopt.getopt(sys.argv[1:], 'bt')
    if len(argv) == 2:
        pathi, patho = argv
        try:
            s = time.time()
            log('Loading...')
            app = ZDic()
            app.pdbName = os.path.splitext(os.path.basename(patho))[0]
            if ('-t', '') in opts:
                log('Processing...')
                app.p2t(pathi, patho)
            else:
                app.fromPath(pathi)
                log('Processing...')
                app.resizeBlock()
                log('Saving...')
                app.toPDB(patho)
            cost = time.time() - s
            log('Success! %dh%dm%ds passed.' % (cost/3600, cost%3600/60, cost%60))
        except:
            log('Error!')
            raise      
    else:
        log('Syntax: [-t] file1 file2')


