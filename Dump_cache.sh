import memcache
import sys

def dump_memcached(host='localhost', port=11211):
    mc = memcache.Client([f'{host}:{port}'], debug=True)
    stats = mc.get_stats('items')

    for slab_id, slab_data in stats.items():
        if not slab_id.startswith('items:'):
            continue

        slab_id = slab_id.split(':')[1]
        cachedump = mc.get_stats(f'cachedump {slab_id} 1000')

        for key in cachedump.get(f'items:{slab_id}:cachedump', {}).keys():
            value = mc.get(key)
            print(f"Clé: {key}")
            print(f"Valeur: {value}")
            print("---")

if __name__ == "__main__":
    dump_memcached()

