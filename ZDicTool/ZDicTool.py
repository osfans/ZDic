#!/usr/bin/env python
# -*- coding: utf-8 -*-
U"""
ZDic转换工具
可将ZDic标准文本TXT、维基XML或BZ2压缩格式、金山词霸DA3格式转换为PDB词典格式
也可将PDB格式转换为文本格式
本脚本使用UTF-8格式编码
"""

import os, sys, getopt, bz2, time, datetime, re, htmlentitydefs, bsddb, string
from struct import *
from xml.dom.minidom import parseString
from xml.parsers.expat import ExpatError

enc = sys.getfilesystemencoding()                       #系统默认编码
enc = 'gbk' if enc == 'UTF-8' else enc                  #如果是UTF-8，则改为GBK，否则不变

bsddb_opt = False                                       #默认不采用bsddb保存
block_size = 1024 * 32 #读入块大小

replace_ste = [('<hr>', '//STEHORIZONTALLINE//\\n'),
               ('<center>', '//STECENTERALIGN//'), ('</center>','\\n'),
               ('<u>', '//STEHYPERLINK='), ('</u>', '\1//'),
               ('<i>', '//STEGRAYFONT//'), ('</i>', '//STECURRENTFONT//'),
               ('<b>', '//STEBOLDFONT//'), ('</b>', '//STESTDFONT//'),
               ('<big>', '//STEBOLDFONT//'), ('</big>', '//STESTDFONT//'),
               ('<sup>', '//STEREDFONT//'), ('</sup>', '//STECURRENTFONT//'),
               ('<small>', '//STEGRAYFONT//'), ('</small>', '//STECURRENTFONT//')
              ] #可替换成STE的HTML标记

def getData(page, title):
    U"获取相应字段数据"
    try:
        return page.getElementsByTagName(title)[0].childNodes[0].data
    except:
        return ''

def ste(s):       
    for i, j in replace_ste: #替换HTML标记
        s = s.replace(i, j)     
    s = re.sub('\[\[([^\[/]*?)\]\]', '//STEHYPERLINK=\g<1>\1//', s) #把[[]]变成STE超链接
    return s

def unste(s):    
    return s.replace('//STEHYPERLINK=', '[[').replace('\1//', ']]')#把STE超链接还原成[[]]
                    
class ZDic:
    def __init__(self, compressFlag = 2, recordSize = 0x4000):
        U"初始化ZDic类，默认为Zlib压缩方式，16K页面大小"
        self.index = ''
        self.byteLen = []
        self.lenSec = ''
        self.creatorID = 'Kdic'
        self.pdbType = 'Dict'
        self.maxWordLen = 32
        self.recordSize = recordSize        
        self.compressFlag = compressFlag
        self.PDBHeaderStructString = '>32shhLLLlll4s4sllH'           #PDB文件头
        self.PDBHeaderStructLength = calcsize(self.PDBHeaderStructString) #PDB文件头长度
        self.PDBHeaderPadding = '\0\0'                                     #PDB文件头补白
    
    def fromPDB(self, path):
        U"从PDB文件中读取数据"
        f = open(path, 'rb')
        self.header = f.read(self.PDBHeaderStructLength)
        self.pdbName = self.header[:32].rstrip('\0')
        self.bnum = unpack('>l',self.header[-4:])[0]
        startOffset = unpack('>L', f.read(4))[0]
        self.lines = {}
        for i in range(self.bnum - 1):
            print '\b\b\b\b\b%3d%%' % (float(i*100)/self.bnum),
            f.seek(self.PDBHeaderStructLength + (i + 1) * 8)
            endOffset = unpack('>L', f.read(4))[0]
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

    def fromWIKI(self, path):
        U"从WIKi XML或者xml.bz2中读取数据"
        ext = os.path.splitext(path)[1].lower() 
        replace_dic = {}                                        #保存需要转换的HTML命名字符，如&nbsp; &lt;等
        for name, codepoint in htmlentitydefs.name2codepoint.items():
            try:
                char = unichr(codepoint).encode(enc)            #enc编码可显示该HTML对象时，才进行替换
                replace_dic['&%s;' % name.lower()] = char
            except:
                pass
        removed_title = ['Help', 'Template', 'Image', 'Portal', 'Wikipedia', 'MediaWiki', 'WP'] #要删除的WIKI词条
        
        def trim(s):
            U"处理XML文件标记"
            s = s.encode(enc, 'replace').replace('&amp;', '&')   #把UTF8编码转换为enc编码，无法编码的用?代替
            for name, char in replace_dic.iteritems():          #替换HTML命名字符
                s = s.replace(name, char)
            s = re.compile('(</?(p|font|br|tr|td|table|div|span|ref).*?>)|(\[\[[a-z]{2,3}(-[a-z]*?)?:[^\]]*?\]\])|(<!--.*?-->)',\
                            re.I|re.DOTALL).sub('', s)          #替换部分HTML标记
            s = re.sub('\n{3,}','\n\n',s).replace('\n', r'\n')  #把连续的多个换行符替换成两个换行符
            return s
        
        f = bz2.BZ2File(path) if ext == '.bz2' else open(path)  #打开压缩或者不压缩格式均可
        percent = float(block_size)/os.path.getsize(path)
        process = percent
        s = f.read(block_size)
        while 1:
            try:
                start = s.index('<page>')
                end = s.index('</page>',start + 6) + 7                #获取page段
                dom = parseString(s[start:end])
                word, mean = trim(getData(dom, 'title')), trim(getData(dom, 'text'))
                if ((word in self.lines) and (mean[:9].lower()=='#redirect')) \
                   or ((':' in word) and word[:word.index(':')] in removed_title):#跳过简繁转换后的重复词条或跳过某些类别词条
                    pass
                elif word in self.lines:
                    self.lines[word]+=r'\n\n\n' + ste(mean) #重复词条，合并成一条
                else:
                    self.lines[word]=ste(mean)
                s = s[end:]
            except:
                tmp = f.read(block_size)
                process += percent
                print '\b\b\b\b\b%3d%%' % (process*100),
                if tmp:
                    s += tmp
                else:
                    break
        f.close()

    def fromDIC(self, path):
        U"从金山词霸dic格式的转换为raw格式的da3文件，再调用fromDA3"
        print
        os.system('KSDrip.exe %s tmp /raw' % path)
        self.fromDA3(os.path.join('tmp', 'tmp.da3'))

    def fromDA3(self, path):
        U"从金山词霸raw格式的da3文件中读取数据"
        
        self.dic_fmt={'YX':'//STEBOLDFONT//%s//STESTDFONT//', 'DX':'//STEBOLDFONT//%s//STESTDFONT//', #粗体
                 'JX':'*%s',                                    #列表
                 'RP':'//STEORANGEFONT//%s//STECURRENTFONT//',  #假名                                 #音标
                 'LY':'//STELEFTINDENT=10////STEREDFONT//%s//STECURRENTFONT////STELEFTINDENT=0//', 'LS':'//STELEFTINDENT=10////STEBLUEFONT//%s//STECURRENTFONT////STELEFTINDENT=0//', 
                 }

        self.dic_head = {'CY':U'基本词义',
                    'YF':U'用法',
                    'JC':U'继承用法',
                    'TS':U'特殊用法',
                    'XB':U'词性变化',
                    'PS':U'派生',
                    'YY':U'语源',
                    'XY':U'习惯用语',
                    'CC':U'参考词汇',
                    'CZ':U'常用词组'
                    }
        self.gmx = U"""\
 ɪɛæɑ ɔ    θ  ʊʌəˌɜɚɝṃṇḷ…ŋɒ ðʃʒˌ\
 !"#$%&ˈ()*+，-．/0123456789ː；<=>?\
@ABCDEFGHIJKLMNOPQʀSTUVWXYZ[\]^_\
`abcdefɡhijklmnopqrstuvwxyz{|}̃ \
€ ‚ƒ„…†‡ˆ‰Š‹Œ    ‘’“”•–—˜™š›œ  Ÿ\
 ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶•¸¹º»¼½¾¿\
ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØǜǘǚǖüÞß\
àáǎãāåæçèéěēìíǐīðñòóǒõō÷øùúǔūýþÿ"""
        self.gmx_allo = dict(zip(U'βɵ∫Ʒａɐêëɨô', U'ßθʃʒaɒÄÊËÔ'))    #可转换为已有GMX字符
        self.gmx_trans = {   U'法':'FR',
                        U'德':'DE',
                        U'俄':'RU',
                        U'意':'IT',
                        U'匈':'HU',
                        U'爱':'IE',
                        U'荷':'NL',
                        U'法':'FR',
                        U'德':'DE',
                        U'美':'US',
                        U'英':'EN',
                        U'ʤ':'d\x1e'
                        }
        self.patternB = re.compile('&[Bb]\{([^\}]*?)\}') #粗体
        self.patternB2 = re.compile('\}(.*?)&[Bb]\{') #粗体2，错乱的标记，前后调整一下
        self.patternB3 = re.compile('&[Bb]\{(.*?)\}') #粗体3，错乱的标记，删除
        self.patternI = re.compile('&[Ii]\{([^\}]*?)\}') #斜体
        self.patternL = re.compile('&[Ll]\{([^\}]*?)\}') #链接
        self.patternP = re.compile('&\+\{([^\}]*?)\}')   #上标
        self.patternR = re.compile('<.*?>')          #删除

        def trim(s):
            s = s.replace('\n', r'\n')
            s = self.patternI.sub('//STEGRAYFONT//\g<1>//STECURRENTFONT//', s)
            s = self.patternL.sub('//STEHYPERLINK=\g<1>\1//', s)
            s = self.patternP.sub('//STEREDFONT//\g<1>//STECURRENTFONT//', s)
            s = self.patternB.sub('//STEBOLDFONT//\g<1>//STESTDFONT//', s)
            s = self.patternB2.sub('//STEBOLDFONT//\g<1>//STESTDFONT//', s)
            s = self.patternB3.sub('\g<1>', s) 
            s = self.patternR.sub('', s)
            return s

        self.palm_trans=re.compile('&#(1[6-9][0-9]|2[0-4][0-9]|25[0-5]);')

        def trimWord(s):
            "词条中&#161-255;换成palm可显示"
            return self.palm_trans.sub(lambda x:chr(int(x.group(1)))+ ' ', s)

        def transform(dom):
            U"""
            &B{粗体}, &I{斜体}, &+{上标}, &L{链接}
            CK:词库
                DC:词头 生词本
                JS:解释
                    CY:基本词义
                    YF:用法
                    JC:继承用法
                    TS:特殊用法
                    XB:词性变化
                    PS:派生
                    YY:语源
                    XY:习惯用语
                    CC:参考词汇
                    CZ:常用词组
                        CX:
                            YX:语头* 发音
                            未使用之文本，DX
                            DX:词性、说明 粗体
                                [infg:派生
                                    sy:单复数
                                    inf:后缀
                                sc:世纪
                                lg:
                                ge:]
                            YJ:
                            GZ:*
                            OJ:又作
                            YD:同形词 后带上标
                            YB:音标
                                RP:日语假名
                                CB:音标
                                PY:拼音
                            JX:* 列表
                            GZ:
                            LJ:* 蓝色缩进
                                LY:原
                                LS:释                    
                            GT:
                                GY:原
                                GE:解
                            XG:同义词
            """
            for node in dom.childNodes:
                if node.nodeType==node.CDATA_SECTION_NODE:
                    node.data = trim(node.data)
                    if dom.tagName in ['CB', 'PY']:
                        if node.data.startswith('&x{'):
                            node.data = node.data[3:-1]
                        tmpipa=''
                        try:
                            for c in node.data:
                                if c in string.printable:
                                    tmpipa += c.encode(enc)
                                elif c in self.gmx:
                                    tmpipa += chr(self.gmx.index(c))
                                elif c in self.gmx_allo:
                                    tmpipa += chr(self.gmx.index(self.gmx_allo[c]))
                                elif c in self.gmx_trans:
                                    tmpipa += self.gmx_trans[c]
                                else:
                                    raise
                            self.mean += '[%s]' % tmpipa + r'\n'
                        except:
                            self.mean += node.data.encode(enc, 'xmlcharrefreplace') + r'\n'
                            self.noipa += c                
                    else:
                        node.data = node.data.encode(enc, 'xmlcharrefreplace')  #enc编码
                        if dom.tagName=='DC':
                            self.word = node.data                                   #词条名
                        elif dom.tagName=='YX':
                            if self.mean or (not(self.mean) and self.word != node.data):
                                self.mean += self.dic_fmt[dom.tagName] % node.data + r'\n' #与词条名不同则添加
                        elif dom.tagName in self.dic_fmt:
                            self.mean += self.dic_fmt[dom.tagName] % node.data + r'\n' #格式化
                        else:
                            self.mean += node.data + r'\n'
                elif node.nodeType==node.ELEMENT_NODE:          
                    if node.tagName in self.dic_head and self.mean:#大标题中文名
                        self.mean += '\\n//STEBLUEFONT////STEBOLDFONT//%s//STECURRENTFONT////STESTDFONT//\\n' % \
                                     self.dic_head[node.tagName].encode(enc)
                    transform(node)

        file_enc = 'UTF-16LE'
        tag_start = '<CK>'.encode(file_enc)
        tag_end = '</CK>'.encode(file_enc)
        tag_len = len(tag_end)
        self.noipa = ''                      #记录GMX中没有的音标
        error_da3_filename = U'%s 格式错误的.da3' % path.split('.',1)[0]
        error_ipa_filename = U'%s 无法显示的音标.txt' % path.split('.',1)[0]
        percent = float(block_size)/os.path.getsize(path) 
        f = open(path, 'rb')
        s = f.read(block_size)        
        process = percent      
        while 1:
            try:
                start = s.index(tag_start)
                end = s.index(tag_end, start) + tag_len                
                doc = s[start:end].decode(file_enc)
                try:
                    dom = parseString(doc.encode('U8')).childNodes[0]
                    self.word, self.mean = '', ''
                    transform(dom)                          #XML转换
                    self.lines[trimWord(self.word)] = self.mean[:-2]
                except ExpatError, err:
                    open(error_da3_filename, 'ab').write(err.message.encode(file_enc)+doc.encode(file_enc)+'\n\n'.encode(file_enc))
                except:
                    raise
                s = s[end:]
            except ValueError:
                tmp = f.read(block_size)
                process += percent
                print '\b\b\b\b\b%3d%%' % (process*100), 
                if tmp:
                    s += tmp
                else:
                    break
            except:
                raise
            
        f.close()
        if self.noipa:
            open(error_ipa_filename,'w').write((U'这些音标未在GMX中找到匹配字符：%s'% (''.join(set(self.noipa)))).encode('U8'))

    def fromTXT(self, path):
        U"从TXT中读取数据"
        f=open(path,'rU')
        for i in f:
            i = i.rstrip().replace(' /// ', '\t', 1)    #如果使用 /// 分隔，则替换成\t
            if '\t' in i:
                word, mean = i.split('\t', 1)           #分隔词语和释义
                if word in self.lines:
                    self.lines[word] += r'\n\n\n' + ste(mean) #重复词条，合并成一条
                else:
                    self.lines[word] = ste(mean)   #STE处理
        f.close()

    def fromPath(self, path):
        U"读取文件"
        files = [] #保存所有要转换的文件       
        if os.path.isdir(path):
            files = glob.glob(os.path.join(path, '*.*'))
        else:
            files = [path] 
        self.lines = bsddb.btopen('tmp.db','c') if bsddb_opt else {}    #是否使用bsddb
        for path in files:
            print 'Init',
            ext = os.path.splitext(path)[1].upper()                     #获取后缀名
            if ext in ['.XML', '.BZ2']:                                 #WIKI XML
                self.fromWIKI(path)
            elif ext in ['.DA3', '.PDB', '.DIC', '.TXT']:
                eval('self.from%s(path)' % ext[1:])
            else:                                                       #默认为TXT
                self.fromTXT(path)
            print '\b\b\b\b\bDone',
        if bsddb_opt:
            self.lines.sync()
            self.lines.close()            

    def resizeBlock(self):
        tmp=''
        first=''
        f=open('tmp.pdb','wb')
        if '' not in self.lines: #判断有无词典信息，没有则自动添加
            self.lines[''] = ste('<b>Welcome to //STEPURPLEFONT//%s//STECURRENTFONT//!</b>\\n<b>Words count:</b> //STEBLUEFONT//%d//STECURRENTFONT//\\n<b>Made time:</b> //STEREDFONT//%s'%(self.pdbName,len(self.lines),time.strftime("%Y.%m.%d")))
        lnum = len(self.lines)
        count = 0
        print 'Init',
        for word in sorted(self.lines.keys()):
            count += 1
            print '\b\b\b\b\b%3d%%' % (float(count)/lnum*100),
            line = "%s\t%s\n"%(word,self.lines[word])
            if first=='':
                first=tmp
            if len(tmp)+len(line)<self.recordSize:
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
                            try:
                                breakindex = line.rindex('\\n', 0, blen)#在换行符处截断
                                tmpline = line[:breakindex]
                                line = line[breakindex+2:]
                            except:
                                tmpline = line[:blen]
                                line = line[blen:]
                        self.compress("%s\t%s\n"%(tmpindex,tmpline),tmpindex,tmpindex,f)
                        i+=1
                else:
                    tmp=line
                first=''
        if tmp:
            self.compress(tmp,first.split('\t',1)[0],line.rsplit('\n',2)[-2].split('\t',1)[0],f)
        f.close()
        print '\b\b\b\b\bDone',

    def resizeBlockB(self):
        """bsddb version by emfox"""
        tmp=''
        first=''
        f=open('tmp.pdb','wb')
        dbf = bsddb.btopen('tmp.db','w')
        if '' not in dbf:
            dbf[''] = ste('<b>Welcome to //STEPURPLEFONT//%s//STECURRENTFONT//!</b>\\n<b>Words count:</b> //STEBLUEFONT//%d//STECURRENTFONT//\\n<b>Made time:</b> //STEREDFONT//%s'%(self.pdbName,len(dbf),time.strftime("%Y.%m.%d")))
        word,mean = dbf.first()
        while 1:
            line = "%s\t%s\n"%(word,mean)
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
                            """if '//STE' in line[blen-40:blen+4]: #split form //STE
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
                                tmpline=tmpline[:-1]"""
                            try:
                                breakindex = line.rindex('\\n', 0, blen)#在换行符处截断
                                tmpline=line[:breakindex]
                                line=line[breakindex+2:]
                            except:
                                tmpline=line[:blen]
                                line=line[blen:]                            
                        self.compress("%s\t%s\n"%(tmpindex,tmpline),tmpindex,tmpindex,f)
                        i+=1
                else:
                    tmp=line
                first=''
            try:
                word,mean = dbf.next()
            except:
                break
        if tmp:
            self.compress(tmp,first.split('\t',1)[0],line.rsplit('\n',2)[-2].split('\t',1)[0],f)
        dbf.close()
        f.close()
        os.remove('tmp.db')
            
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
        return pack(self.PDBHeaderStructString,
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
        offset=self.PDBHeaderStructLength + 8 * self.bnum +len(self.PDBHeaderPadding)
        uid=0x406F8000      
        for i in self.byteLen:
            towrite += pack('>2L', offset, uid) 
            offset += i
            uid += 1
        del offset,uid,i,self.byteLen
        towrite += self.PDBHeaderPadding
        f = open(path,'wb')
        #header and index
        f.write(towrite+self.index)
        del towrite, self.index
        percent = float(block_size)/os.path.getsize('tmp.pdb')
        process = 0
        print '%3d%%' % 0,
        tmp = open('tmp.pdb','rb')
        while 1:
            s=tmp.read(block_size)
            process += percent
            print '\b\b\b\b\b%3d%%' % (process*100),
            if s:
                f.write(s)
            else:
                break
        tmp.close()
        print '\b\b\b\b\bDone',
        os.remove('tmp.pdb')
        f.close()

    def p2t(self, path, patho):
        U"从PDB文件快速转换为TXT文件，不需要排序"
        f = open(path, 'rb')        
        f.seek(self.PDBHeaderStructLength - 4)
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
            print 'Init',
            for i in range(1, self.bnum - 1):
                print '\b\b\b\b\b%3d%%' % (float(i)/self.bnum*100),
                f.seek(self.PDBHeaderStructLength + (i + 1) * 8)
                endOffset = unpack('>L',f.read(4))[0]
                f.seek(startOffset)
                t.write(unste(f.read(endOffset - startOffset).decode('zlib')))
                startOffset = endOffset
            f.seek(startOffset)
            t.write(unste(f.read().decode('zlib')))
            t.close()
            f.close()
            print '\b\b\b\b\bDone',

    def toTXT(self, patho):
        U"将lines按序保存至txt文件中"
        f = open(patho, 'w')
        lnum = len(self.lines)
        count = 0
        print '%3d%%' % count,
        for word, mean in sorted(self.lines.iteritems()):
            count += 1
            print '\b\b\b\b\b%3d%%' % (float(count)/lnum*100),
            print >>f, "%s\t%s" % (word, unste(mean))
        print '\b\b\b\b\bDone',
        f.close()        

def log(msg):
    U"打印日志"
    print '[%s]%s'%(time.strftime('%X'), msg),
    
if __name__ == '__main__':   
    opts, argv = getopt.getopt(sys.argv[1:], 'bt')
    if len(argv) != 2:
        log(U'Syntax: [-b] [-t] filename1 filename2') #参数错误时给出语法提示
    else:
        pathi, patho = argv
        try:
            s = time.time() #记录开始时间
            log(U'Loading...')
            app = ZDic()    #初使化ZDic数据结构
            app.pdbName = os.path.splitext(os.path.basename(patho))[0]
            if patho[-3:].lower() == 'txt' or ('-t', '') in opts: #目标文件为文本文件时，转换为文本文件
                if pathi.endswith('pdb'):
                    app.p2t(pathi, patho)
                    print
                else:
                    app.fromPath(pathi)
                    print
                    log(U'Saving...')
                    app.toTXT(patho)
                    print
            else:
                bsddb_opt = ('-b', '') in opts #-b参数：使用bsddb，较小内存，较大文件，较长时间
                app.fromPath(pathi)
                print
                log(U'Processing...')
                if bsddb_opt:
                    app.resizeBlockB()
                else:
                    app.resizeBlock()
                print
                log(U'Saving...')
                app.toPDB(patho)
                print
            cost = time.time() - s #计算花费时间
            log(U'Success! It costs %dh%dm%ds.\n' % (cost/3600, cost%3600/60, cost%60))
        except:
            log(U'Error!\n')
            raise
        


