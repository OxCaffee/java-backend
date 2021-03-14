# Nginx负载均衡与HTTP反向代理

## 1. 为什么需要负载均衡和反向代理

对于反向代理而言，目前绝大多数应用在面临高流量用户访问请求的时候都采用集群的方式，将服务器从一台扩容到几百台上千台。然而对于用户来说，在浏览器的地址栏输入域名(例如`www.baidu.com`)，每次访问的可能都不是同一个服务器，但是这些多个服务器IP共享一个域名地址，这种**向访问者隐藏服务器，使用户不知道访问的是那一台服务器的做法称为反向代理**，同理，向浏览器隐藏客户端的做法称为**正向代理**。

对于负载均衡而言，当高QS的流量访问同一个域名的时候，如果把全部的流量都导入到一台服务器上，那么结果是致命的，所以负载均衡算法通过一种或者多种特定组合的手段，把请求的访问流量进行合理的可承受的分流到各个服务器上。拿DNS为例，对于外网DNS，流量更倾向于访问就近的DNS服务器，这时候使用的是一种全局的负载均衡算法。当内网DNS的时候，使用的是简单的轮询负载均衡。

## 2. 如何配置upstream实现负载均衡

Nginx实现负载均衡的方式主要有三种种，一种是**轮询(round-robin)**，另外一种是**ip_hash(根据客户IP实现负载均衡)**，一般二者会搭配起来使用，还有一种是**hash key [consistent]（哈希一致性算法)**下面简单介绍一下Nginx三种负载均衡算法的基本使用场景：

* **当携带的流量请求没有session会话的时候，简单轮询即可**

```nginx
upstream backend{
    server 192.168.1.101:9080;
    server 192.168.1.101:9090;
}
```

* **当携带的流量请求含有session会话，为了保存之前的会话信息，需要将用户重定向到上一次访问的服务器**

```nginx
upstream backend{
    ip_hash;
    server 192.168.1.101:9080;
    server 192.168.1.101:9090;
}
```

* **当upstream server性能不均的时候，需要设置访问的权值**

```nginx
upstream backend{
    server 192.168.1.101:9080 weight=1;
    server 192.168.1.101:9090 weight=2;
}
```

* **当添加或者删除某一个服务器的时候，可能会导致很多key被负载均衡到不同的服务器，从而导致业务出错，这时候需要使用哈希算法，保证当添加或者删除一台服务器的时候，只有少数的key被负载均衡到不同的服务器上**

```nginx
upstream backend{
    hash $uri;
    server 192.168.1.101:9080 weight=1;
    server 192.168.1.101:9090 weight=2;
}
```

* **和上面的配置类似，但是一致性哈希算法可以动态指定consistent_key**

```nginx
upstream backend{
    hash $consistent_key consistent;
    server 192.168.1.101:9080 weight=1;
    server 192.168.1.101:9090 weight=2;
}
```

可以通过location指定一致性hash key，此处会优先考虑请求参数cat，如果请求参数中没有cat，再根据请求uri进行负载均衡

```nginx
location / {
    set $consistent_key $arg_cat
        if($consistent_key = ""){
        set $consistent_key $request_uri;
    }
}
```

然而在大多数生产环境中，我们是通过Lua设置一致性哈希key

```lua
set_by_lua_file $consistent_key "lua_balancing.lua";

--lua_balancing.lua
local consistent_key = args.cat
if not consistent_key or consistent_key == "" then
    consistent_key = ngx_var.request_uri
end

local value = balancing_cache:get(consistent_key)
if not value then
    success, err = balancing_cache:set(consistent_key, 1, 60)
else
    newval, err = balancing_cache:incr(consistent_key, 1)
end
```

如果某一个分类请求量太大，上游服务器无法处理，可以在一致性hash key后面加上计数实现类似轮询的算法

```nginx
if new_val > 5000 then
	consistent_key = consistent_key .. '_' .. new_val
end
```

## 3. Nginx上游服务器请求失败重试

主要配置两个方面: `upstream server`和`proxy_pass`

```nginx
upstream backend{
    server 192.168.1.101:9080 max_fails=2 fail_timeout=10s weight=1;
    server 192.168.1.101:9090 max_fails=2 fail_timeout=10s weight=2;
}
```

当上游服务器在`fail_timeout`时间段内失败了`max_fails`次请求，则认为该上游服务器不可用。不可活，然后摘掉这个上游服务器，`fail_timeout`时间后会再次将该服务器加入到存活上游服务器列表中进行重试

```nginx
local /test {
    proxy_connect_timeout 5s;
    proxy_read_timeout 5s;
    proxy_send_timeout 5s;
    
    proxy_next_upstream error timeout;
    proxy_next_upstream_timeout 10s;
    proxy_next_upstream_tries 2;
    proxy_pass http://backend;
    add_header upstream_addr $upstream_addr
}
```

然后进行`proxy_next_upstream`相关配置，当遇到配置错误的时候，会重试下一台服务器。

