import base64
import hmac
import time
from urllib.parse import quote

def token(id,access_key):
    version = '2018-10-31'
    res = 'products/%s' % id
    et = str(int(time.time()) + 315360000)
    method = 'sha1'
    key = base64.b64decode(access_key)
    org = et + '\n' + method + '\n' + res + '\n' + version
    sign_b = hmac.new(key=key, msg=org.encode(), digestmod=method)
    sign = base64.b64encode(sign_b.digest()).decode()
    sign = quote(sign, safe='')
    res = quote(res, safe='')
    token = 'version=%s&res=%s&et=%s&method=%s&sign=%s' % (version, res, et, method, sign)
    return token
if __name__ == '__main__':
    id = '388752'
    access_key = 'QNbnj7mS4aOTcNHnQCAEPO/2Chv9yNZOqhghd1fYRkw='

    print(token(id,access_key))
